# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

"""
Fabric definitions for HemeLB
Usage:
        With current working directory anywhere inside the HemeLB checkout, execute
                "fab <machinename> <task>"
        for example:
                "fab hector deploy_cold"

        Do fab -l to get a list of available commands.

        Before use, you MUST copy deploy/machines_user_example.yml as deploy/machines_user.yml and fill in your personal details.
        For smoothest usage, you should also ensure ssh keys are in place so the target machine can access the hemelb repository.
"""
from templates import *
from machines import *
from fabric.contrib.project import *
from xml.etree import ElementTree
import time
import re
import numpy as np
import yaml
import tempfile
from os.path import expanduser


@task
def clone():
    """Delete and checkout the repository afresh."""
    run(template("mkdir -p $remote_path"))
    if env.no_ssh or env.no_git:
        with cd(env.remote_path):
            run(template("rm -rf $hemelb_repo"))
        # Some machines do not allow outgoing connections
        # so the data must be sent by a project sync instead.
        execute(sync)
         # On such machines, we cannot rely on an outgoing connection to servers to find dependencies either.
    else:
        with cd(env.remote_path):
            run(template("rm -rf $hemelb_repo"))
            run(template("git clone $github:$github_user/$hemelb_repo"))
        with cd(env.repository_path):
            with prefix(env.build_prefix):
                run("git rev-parse HEAD > revision_info.txt")
    if env.no_ssh or env.needs_tarballs:
        execute(send_distributions)

@task
def clone_with_regression_tests():
    execute(clone)
    execute(clone_regression_tests)

@task(alias='cold')
def deploy_cold():
    """Checkout, build, and install hemelb, from new."""
    execute(clear_build)
    execute(clone)
    execute(prepare_paths)
    execute(configure)
    execute(build)
    execute(install)
    execute(build_python_tools)

@task
def update_build():
    """Update and do incremental build."""
    execute(update)
    execute(require_recopy)
    execute(configure)
    execute(build)
    execute(install)
    execute(build_python_tools)

@task
def require_recopy():
    """Notify the build system that the code has changed."""
    run(template("rm -rf $build_path/hemelb-prefix"))

@task
def update():
    """Update the remote repository"""
    if env.no_ssh or env.no_git:
        execute(sync)
    else:
        with cd(env.repository_path):
            with prefix(env.build_prefix):
                run("git pull")
        with cd(env.repository_path):
            with prefix(env.build_prefix):
                run("git rev-parse HEAD > revision_info.txt")

@task
def prepare_paths():
    """Create remote locations to store results and configs"""
    run(template("mkdir -p $scripts_path"))
    run(template("mkdir -p $results_path"))
    run(template("mkdir -p $config_path"))
    run(template("mkdir -p $profiles_path"))

@task
def clear_build():
    """Wipe out existing built and installed HemeLB."""
    run(template("rm -rf $build_path"))
    run(template("rm -rf $install_path"))
    run(template("mkdir -p $build_path"))
    run(template("mkdir -p $install_path"))
    run(template("mkdir -p $temp_path"))


@task
def clean():
    """Clean remote build area."""
    with cd(env.build_path):
        run("make clean")

@task(alias='tools')
def build_python_tools():
    """Build and install python scripts."""
    with cd(env.tools_path):
        with prefix(env.build_prefix):
            run(template("python setup.py install --prefix $install_path"))

@task
def stat():
    """Check the remote message queue status"""
    #TODO: Respect varying remote machine queue systems.
    return run(template("$stat -u $username"))

@task
def monitor(delay=30):
    """Report on the queue status, ctrl-C to interrupt"""
    while True:
        execute(stat)
        time.sleep(float(delay))
        
        
def check_complete():
  """Return true if the user has no queued jobs"""
  return stat() == ""
     
@task
def wait_complete():
  """Wait until all jobs currently qsubbed are complete, then return"""
  time.sleep(30)
  while not check_complete():
        time.sleep(30)

def configure_cmake(configurations, extras):
    #Read additional configurations from the available options
    if not configurations:
        configurations = ['default']
    options = {}
    for configuration in configurations:
        options.update(cmake_options[configuration])
    options.update(extras)
    options.update(env.cmake_options)
    options.update({
        'CMAKE_INSTALL_PREFIX': env.install_path,
        'HEMELB_DEPENDENCIES_INSTALL_PATH': env.install_path,
        })
    env.total_cmake_options = options
    env.cmake_flags = ' '.join(["-D%s=%s" % option for option in env.total_cmake_options.iteritems()])

@task
def configure(*configurations, **extras):
    """CMake configure step for HemeLB and dependencies."""
    # Set make parallism for master build
    extras['HEMELB_SUBPROJECT_MAKE_JOBS'] = env.make_jobs
    configure_cmake(configurations, extras)

    with cd(env.build_path):
        with prefix(env.build_prefix):
            run(template("rm -f $build_path/CMakeCache.txt"))
            run(template("cmake $repository_path $cmake_flags"))

@task
def code_only(*configurations, **extras):
    """Configure, build, and install for the /Code C++ code only, do not attempt to install and build dependencies."""
    execute(configure_code_only, *configurations, **extras)
    execute(build_code_only)
    execute(install_code_only)

@task
def configure_code_only(*configurations, **extras):
    """CMake configure step for HemeLB code only."""
    configure_cmake(configurations, extras)
    run(template("rm -rf $code_build_path"))
    run(template("mkdir -p $code_build_path"))
    with cd(env.code_build_path):
        with prefix(env.build_prefix):
            run(template("cmake $repository_path/Code $cmake_flags"))

@task
def build_code_only(verbose=False):
    """CMake build step for HemeLB code only."""
    with cd(env.code_build_path):
        with prefix(env.build_prefix):
            if verbose or env.verbose:
                run(template("make -j$make_jobs VERBOSE=1"))
            else:
                run(template("make -j$make_jobs"))

@task
def install_code_only():
    """CMake install step for HemeLB code only."""
    with cd(env.code_build_path):
        with prefix(env.build_prefix):
            run("make install")
            # run(template("chmod u+x $install_path/bin/unittests_hemelb $install_path/bin/hemelb"))
            run(template("cp $code_build_cache $build_cache")) # So that the stored cmake cache matches the active build

@task
def build(verbose=False):
    """CMake build step for HemeLB and dependencies."""
    with cd(env.build_path):
        run(template("rm -rf hemelb_prefix/build"))
        with prefix(env.build_prefix):
            if verbose or env.verbose:
                run("make -j$make_jobs VERBOSE=1")
            else:
                run("make -j$make_jobs")

@task
def install():
    """CMake install step for HemeLB and dependencies."""
    with cd(env.build_path):
        with prefix(env.build_prefix):
            #run("make install") // Doesn't have a separate install step, as HemeLB gets installed by the sub-project to the final install dir.
            run(template("chmod u+x $install_path/bin/unittests_hemelb $install_path/bin/hemelb"))

@task
def reset(args=None):
    """Revert local changes in the remote repository.
     Including those made through 'fab ... patch' and 'fab ... sync'.
    Specify a path relative to the repository root to revert only some files or directories
    """
    if args is None:
        command = "git reset --hard"
    else:
        command = "git checkout %s" % args
        
    with cd(env.repository_path):
        run(command)

@task
def send_distributions():
    """Transmit dependency tarballs to remote.
    Files taken from dependencies/distributions.
    Useful to prepare a build on target machines with CMake before 2.8.4, where
    HTTP redirects are not followed.
    """
    run(template("mkdir -p $repository_path/dependencies/distributions"))
    rsync_project(local_dir=os.path.join(env.localroot, 'dependencies', 'distributions') + '/',
    remote_dir=env.pather.join(env.repository_path, 'dependencies', 'distributions'))

@task
def fetch_distributions():
    """Fetch dependency tarballs tfrom remote.
    Files taken from dependencies/distributions.
    Useful to prepare a build on target machines with CMake before 2.8.4, where
    HTTP redirects are not followed.
    """
    local(template("rsync -pthrvz $username@$remote:$repository_path/dependencies/distributions/ $localroot/dependencies/distributions"))

@task
def clone_regression_tests():
    """Get the latest data from the repo."""
    if env.no_ssh or env.no_git:
        # Some machines do not allow outgoing connections
        # so the data must be sent by a project sync instead.
        execute(sync_regression_tests)
    else:
        run(template("if [ -d $regression_test_repo_path ]; then cd $regression_test_repo_path && git pull $github:$github_user/$regression_tests_repo; else mkdir -p $regression_test_repo_path && cd $regression_test_repo_path && git clone $github:$github_user/$regression_tests_repo . ; fi"))

@task
def copy_regression_tests():
    if env.regression_test_source_path != env.regression_test_path:
        run(template("cp -r $regression_test_source_path $regression_test_path"))

@task
def sync():
    """Update the remote repository with local changes.
    Uses rysnc.
    Respects the local .gitignore files to avoid sending unnecessary information.
    """
    rsync_project(
            remote_dir=env.repository_path,
            local_dir=env.localroot + '/',
            exclude=map(lambda x: x.replace('\n', ''),
            list(open(os.path.join(env.localroot, '.gitignore'))) + 
            ['.git']
            )
    )
    # In the case of a sync (non-git) remote, we will not be able to run git on the remote to determine which code is being built.
    # We will therefore assume the id for the current repository, and store that in a separate file along with the code.
    revision_info_path = os.path.join(env.localroot, 'revision_info.txt')
    with open(revision_info_path, 'w') as revision_info:
        revision_info.write(env.build_number)
    put(revision_info_path, env.repository_path)

@task
def sync_regression_tests():
    """Update the remote repository with local changes.
    Uses rysnc.
    Respects the local .gitignore files to avoid sending unnecessary information.
    """
    run(template("mkdir -p $regression_test_repo_path"))
    rsync_project(
            remote_dir=env.regression_test_repo_path,
            local_dir=env.regression_tests_root + '/',
            exclude=map(lambda x: x.replace('\n', ''),
            list(open(os.path.join(env.localroot, '.gitignore'))) + 
            ['.git']
            )
    )

@task
def patch(args=""):
    """Update the remote repository with local changes.
    Uses git diff to generate patchfiles.
    Specify a path relative to the repository root to patch only some files or directories
    e.g 'fab legion patch Code/main.cc'
    """
    local("git diff %s> fabric.diff" % args)
    put("fabric.diff", env.pather.join(env.remote_path, env.repository))
    with cd(env.repository_path):
        run("patch -p1 < fabric.diff")

def with_template_job():
    """
    Determine a generated job name from environment parameters, and then define additional environment parameters based on it.
    """
    name = template(env.job_name_template)
    if env.get('label'):
        name = '_'.join((env['label'], name))
    with_job(name)

def with_job(name):
    """Augment the fabric environment with information regarding a particular job name.
    Definitions created:
    job_results: the remote location where job results should be stored
    job_results_local: the local location where job results should be stored
    """
    env.name = name
    env.job_results = env.pather.join(env.results_path, name)
    env.job_results_local = os.path.join(env.local_results, name)
    env.job_results_contents = env.pather.join(env.job_results, '*')
    env.job_results_contents_local = os.path.join(env.job_results_local, '*')


def with_template_config():
    """
    Determine the name of a used or generated config from environment parameters, and then define additional environment parameters based on it.
    """
    with_config(template(env.config_name_template))

def with_config(name):
    """Internal: augment the fabric environment with information regarding a particular configuration name.
    Definitions created:
    job_config_path: the remote location where the config files for the job should be stored
    job_config_path_local: the local location where the config files for the job may be found
    """
    env.config = name
    env.job_config_path = env.pather.join(env.config_path, name)
    env.job_config_path_local = os.path.join(env.local_configs, name)
    env.job_config_contents = env.pather.join(env.job_config_path, '*')
    env.job_config_contents_local = os.path.join(env.job_config_path_local, '*')

def with_profile(name):
    """Internal: augment the fabric environment with information regarding a particular profile name.
    Definitions created:
    job_profile_path: the remote location where the profile should be stored
    job_profile_path_local: the local location where the profile files may be found
    """
    env.profile = name
    env.job_profile_path = env.pather.join(env.profiles_path, name)
    env.job_profile_path_local = os.path.join(env.local_profiles, name)
    env.job_profile_contents = env.pather.join(env.job_profile_path, '*')
    env.job_profile_contents_local = os.path.join(env.job_profile_path_local, '*')

@task
def fetch_configs(config=''):
    """
    Fetch config files from the remote, via rsync.
    Specify a config directory, such as 'cylinder' to copy just one config.
    Config files are stored as, e.g. cylinder/config.dat and cylinder/config.xml
    Local path to use is specified in machines_user.json, and should normally point to a mount on entropy,
    i.e. /store4/blood/username/config_files
    This method is not intended for normal use, but is useful when the local machine cannot have an entropy mount,
    so that files can be copied to a local machine from entropy, and then transferred to the compute machine,
    via 'fab entropy fetch_configs; fab legion put_configs'
    """
    with_config(config)
    local(template("rsync -pthrvz $username@$remote:$job_config_path/ $job_config_path_local"))

@task
def put_configs(config=''):
    """
    Transfer config files to the remote.
    For use in launching jobs, via rsync.
    Specify a config directory, such as 'cylinder' to copy just one configuration.
    Config files are stored as, e.g. cylinder/config.dat and cylinder/config.xml
    Local path to find config directories is specified in machines_user.json, and should normally point to a mount on entropy,
    i.e. /store4/blood/username/config_files
    If you can't mount entropy, 'fetch_configs' can be useful, via 'fab entropy fetch_configs; fab legion put_configs'
    """
    with_config(config)
    run(template("mkdir -p $job_config_path"))
    rsync_project(local_dir=env.job_config_path_local + '/', remote_dir=env.job_config_path)

@task
def put_results(name=''):
    """
    Transfer result files to a remote.
    Local path to find result directories is specified in machines_user.json.
    This method is not intended for normal use, but is useful when the local machine cannot have an entropy mount,
    so that results from a local machine can be sent to entropy, via 'fab legion fetch_results; fab entropy put_results'
    """
    with_job(name)
    run(template("mkdir -p $job_results"))
    rsync_project(local_dir=env.job_results_local + '/', remote_dir=env.job_results)

@task
def fetch_results(name='', regex=''):
    """
    Fetch results of remote jobs to local result store.
    Specify a job name to transfer just one job.
    Local path to store results is specified in machines_user.json, and should normally point to a mount on entropy,
    i.e. /store4/blood/username/results.
    If you can't mount entropy, 'put results' can be useful,  via 'fab legion fetch_results; fab entropy put_results'
    """
    with_job(name)
    local(template("rsync -pthrvz $username@$remote:$job_results/%s $job_results_local" % regex))

@task
def clear_results(name=''):
    """Completely wipe all result files from the remote."""
    with_job(name)
    run(template('rm -rf $job_results_contents'))

@task
def fetch_profiles(name=''):
    """
    Fetch results of remote jobs to local result store.
    Specify a job name to transfer just one job.
    Local path to store results is specified in machines_user.json, and should normally point to a mount on entropy,
    i.e. /store4/blood/username/results.
    If you can't mount entropy, 'put results' can be useful,  via 'fab legion fetch_results; fab entropy put_results'
    """
    with_profile(name)
    local(template("rsync -pthrvz $username@$remote:$job_profile_path/ $job_profile_path_local"))

@task
def put_profiles(name=''):
    """
    Transfer result files to a remote.
    Local path to find result directories is specified in machines_user.json.
    This method is not intended for normal use, but is useful when the local machine cannot have an entropy mount,
    so that results from a local machine can be sent to entropy, via 'fab legion fetch_results; fab entropy put_results'
    """
    with_profile(name)
    run(template("mkdir -p $job_profile_path"))
    rsync_project(local_dir=env.job_profile_path_local + '/', remote_dir=env.job_profile_path)

def update_environment(*dicts):
    for adict in dicts:
        env.update(adict)

@task(alias='test')
def unit_test(**args):
    """Submit a unit-testing job to the remote queue."""
    job(dict(script='unittests', job_name_template='unittests_${build_number}_${machine_name}', cores=1, corespernode=1, wall_time='0:1:0', memory='2G'), args)

@task
def batch_build_code(*configurations, **extras):
    """Submit a build job to the remote serial queue."""
    configure_cmake(configurations, extras)
    with settings(batch_header=env.batch_header + '_serial'):
      job(dict(script='batch_build_code', job_name_template='build_${build_number}_${machine_name}', queue='serial', cores=1, wall_time='1:0:0', memory='2G'), extras)

@task
def batch_build(*configurations, **extras):
    """Submit a build job to the remote serial queue."""
    # Set make parallism for master build
    extras['HEMELB_SUBPROJECT_MAKE_JOBS'] = env.make_jobs
    configure_cmake(configurations, extras)
    with settings(batch_header=env.batch_header + '_serial'):
      job(dict(script='batch_build', job_name_template='build_${build_number}_${machine_name}', queue='serial', cores=1, wall_time='1:0:0', memory='2G'), extras)

@task
def hemelb(config, **args):
    """Submit a HemeLB job to the remote queue.
    The job results will be stored with a name pattern as defined in the environment,
    e.g. cylinder-abcd1234-legion-256
    config : config directory to use to define geometry, e.g. config=cylinder
    Keyword arguments:
            cores : number of compute cores to request
            images : number of images to take
            steering : steering session i.d.
            wall_time : wall-time job limit
            memory : memory per node
    """
    with_config(config)
    execute(put_configs, config)
    job(dict(script='hemelb',
            cores=4, images=10, steering=1111, wall_time='0:15:0', memory='2G'), args)
    if args.get('steer', False):
        execute(steer, env.name, retry=True, framerate=args.get('framerate'), orbit=args.get('orbit'))

@task
def multiscale_hemelb(config,**args):
    """Submit a HemeLB multiscale job to the remote queue.
    The job results will be stored with a name pattern as defined in the environment,
    e.g. cylinder-abcd1234-legion-256
    config : config directory to use to define geometry, e.g. config=cylinder
    Keyword arguments:
            cores : number of compute cores to request
            images : number of images to take
            steering : steering session i.d.
            wall_time : wall-time job limit
            memory : memory per node
    """
    with_config(config)
    execute(put_configs,config)
    job(dict(script='multiscale_hemelb',
            cores=4,images=10, steering=1111, wall_time='0:15:0',memory='2G'),args)
    if args.get('steer',False):
        execute(steer,env.name,retry=True,framerate=args.get('framerate'),orbit=args.get('orbit'))


@task
def resubmit(name):
    with_job(name)
    with cd(env.job_results):
        with prefix(env.run_prefix):
            run(template("$job_dispatch ${name}.sh"))

@task
def multijob(*names, **args):
    # We need to put together a job script to submit all the jobs in turn
    # This we do, by just calling the pre-written jobscripts in existing prepared but not run jobs
    jobscriptpaths = ["bash " + env.pather.join(env.scripts_path, name) + ".sh" for name in names]
    env.jobstorun = "\n".join(jobscriptpaths)
    # And then, submit it
    job(dict(script='multijob', job_name_template='multijob',
            cores=4, images=10, steering=1111, wall_time='0:15:0', memory='2G'), args)

@task
def hemelbs(config, **args):
    """Submit multiple HemeLB jobs to the remote queue.
    This can submit a massive number of jobs -- do not use on systems with a limit to number of queued jobs permitted.
    The job results will be stored with a name pattern  as defined in the environment,
    e.g. cylinder-abcd1234-legion-256
    config : config directory to use to define geometry, e.g. config=cylinder
    Keyword arguments:
            cores : number of compute cores to request
            images : number of images to take
            steering : steering session i.d.
            wall_time : wall-time job limit
            memory : memory per node
    """
    for currentCores in input_to_range(args.get('cores'), 4):
        hemeconfig = {}
        hemeconfig.update(args)
        hemeconfig['cores'] = currentCores
        execute(hemelb, config, **hemeconfig)

@task
def hemelb_benchmark(config, min_cores, max_cores, **args):
    """ Performs a strong scaling test from <min_cores> cores to <max_cores> cores, in steps of a factor 2. """
    cores_used = int(min_cores)
    while cores_used < int(max_cores):
        args['cores'] = cores_used
        
        with_config(config)
        execute(put_configs, config)
        job(dict(script='hemelb',
            cores=cores_used, images=10, steering=1111, wall_time='0:15:0', memory='2G'), args)
        cores_used *= 2

@task(alias='regress')
def regression_test(**args):
    """Submit a regression-testing job to the remote queue."""
    execute(clone_regression_tests)
    execute(copy_regression_tests)
    execute(build_python_tools)
    job(dict(job_name_template='regression_${build_number}_${machine_name}', cores=3,
            wall_time='0:20:0', memory='2G', images=0, steering=1111, script='regression'), args)

def calc_nodes():
  # If we're not reserving whole nodes, then if we request less than one node's worth of cores, need to keep N<=n
  env.coresusedpernode = env.corespernode
  if int(env.coresusedpernode) > int(env.cores):
    env.coresusedpernode = env.cores
  env.nodes = int(env.cores) / int(env.coresusedpernode)

# 
def job(*option_dictionaries):
    """Internal low level job launcher.
    Parameters for the job are determined from the prepared fabric environment
    Execute a generic job on the remote machine. Use hemelb, regress, or test instead."""
    env.submit_time = time.strftime('%Y%m%d%H%M%S')
    time.sleep(1.)
    update_environment(*option_dictionaries)
    with_template_job()
    # Use this to request more cores than we use, to measure performance without sharing impact
    if env.get('cores_reserved') == 'WholeNode' and env.get('corespernode'):
        env.cores_reserved = (1 + (int(env.cores) - 1) / int(env.corespernode)) * env.corespernode
    # If cores_reserved is not specified, temporarily set it based on the same as the number of cores
    # Needs to be temporary if there's another job with a different number of cores which should also be defaulted to.
    with settings(cores_reserved=env.get('cores_reserved') or env.cores):
        calc_nodes()
        if env.node_type:
            env.node_type_restriction = template(env.node_type_restriction_template)
        env['job_name'] = env.name[0:env.max_job_name_chars]
        with settings(cores=1):
          calc_nodes()
          env.run_command_one_proc = template(env.run_command)
        calc_nodes()
        env.run_command = template(env.run_command)
        env.job_script = script_templates(env.batch_header, env.script)

        env.dest_name = env.pather.join(env.scripts_path, env.pather.basename(env.job_script))
        put(env.job_script, env.dest_name)
        run(template("touch $build_cache"))
        run(template("mkdir -p $job_results"))
        run(template("cp $dest_name $job_results"))
        run(template("cp $build_cache $job_results"))
        with tempfile.NamedTemporaryFile() as tempf:
            tempf.write(yaml.dump(dict(env)))
            tempf.flush() #Flush the file before we copy it.
            put(tempf.name, env.pather.join(env.job_results, 'env.yml'))
        run(template("chmod u+x $dest_name"))
        # Allow option to submit all preparations, but not actually submit the job
        if not env.get("noexec", False):
                   with cd(env.job_results):
                       with prefix(env.run_prefix):
                           run(template("$job_dispatch $dest_name"))

def input_to_range(arg, default):
    ttype = type(default)
    gen_regexp = "\[([\d\.]+):([\d\.]+):([\d\.]+)\]" #regexp for a array generator like [1.2:3:0.2]
    if not arg:
        return [default]
    match = re.match(gen_regexp, str(arg))
    if match:
        vals = list(map(ttype, match.groups()))
        if ttype == int:
            return range(*vals)
        else:
            return np.arange(*vals)

    return [ttype(arg)]

def load_profile():
    with_profile(env.profile)
    from HemeLbSetupTool.Model.Profile import Profile
    p = Profile()
    try:
        p.LoadFromFile(os.path.expanduser(os.path.join(env.job_profile_path_local, env.profile) + '.pro'))
    except IOError:
        p.LoadFromFile(os.path.expanduser(os.path.join(env.job_profile_path_local, env.profile) + '.pr2'))
    return p

def modify_profile(p):
    #Profiles always get created with 1000 steps and 3 cycles.
    #Can't change it here.
    try:
        p.VoxelSize = type(p.VoxelSize)((env.VoxelSize) or p.VoxelSize)
        env.VoxelSize = p.VoxelSize
        env.StringVoxelSize = str(env.VoxelSize).replace(".", "_")
    except AttributeError:
        # Remain compatible with old setuptool
        p.VoxelSizeMetres = type(p.VoxelSizeMetres)(env.VoxelSize) or p.VoxelSizeMetres
        env.VoxelSize = p.VoxelSizeMetres
    env.Steps = env.Steps or 1000
    env.Cycles = env.Cycles or 3

def profile_environment(profile, VoxelSize, Steps, Cycles, extra_env={}):
    env.profile = profile
    env.VoxelSize = VoxelSize or env.get('VoxelSize')
    try:
        env.VoxelSize = float(env.VoxelSize)
    except ValueError:
        pass
    except TypeError:
        pass
    env.StringVoxelSize = str(env.VoxelSize).replace(".", "_")
    env.Steps = Steps or env.get('Steps')
    env.Cycles = Cycles or env.get('Cycles')
    env.update(extra_env)

def generate(profile):
    if not profile.IsReadyToGenerate:
        raise "Not ready to generate"
    profile.Generate()

def create_config_impl(p):
    modify_profile(p)
    with_template_config()
    p.OutputGeometryFile = os.path.expanduser(os.path.join(env.job_config_path_local, 'config.gmy'))
    p.OutputXmlFile = os.path.expanduser(os.path.join(env.job_config_path_local, 'config.xml'))
    local(template("mkdir -p $job_config_path_local"))
    generate(p)
 
@task
def create_config(profile, VoxelSize=None, Steps=None, Cycles=None, **args):
    """Create a config file
    Create a config file (geometry and xml) based on a configuration profile.
    """
    profile_environment(profile, VoxelSize, Steps, Cycles, args)
    p = load_profile()
    create_config_impl(p)
    # Now modify it to have the specified steps and cycles
    modify_config(profile, VoxelSize, Steps, Cycles, Steps, Cycles)

@task
def modify_config(profile, VoxelSize, Steps=1000, Cycles=3, oldSteps=1000, oldCycles=3):
    """Create a new config by copying an old one, and modifying the steps and cycles"""
    profile_environment(profile, VoxelSize, oldSteps, oldCycles)           
    with_template_config()
    env.old_config_path = env.job_config_path_local
    config_path = os.path.expanduser(os.path.join(env.job_config_path_local, 'config.xml'))
    config = ElementTree.parse(config_path)
    simnode = config.find('simulation')
    for currentSteps in input_to_range(Steps, 1000):
        for currentCycles in input_to_range(Cycles, 3):
            profile_environment(profile, VoxelSize, currentSteps, currentCycles)
            with_template_config()
            if not env.old_config_path == env.job_config_path_local:
                local(template("cp -r $old_config_path $job_config_path_local"))
            new_config_path = os.path.join(env.job_config_path_local, 'config.xml')
            simnode.set('cyclesteps', str(currentSteps))
            simnode.set('cycles', str(currentCycles))
            config.write(new_config_path)

@task
def create_configs(profile, VoxelSize=None, Steps=None, Cycles=None, **args):
    """Create many config files, by looping over multiple voxel sizes, step counts, or cycles
    """
    profile_environment(profile, VoxelSize, Steps, Cycles, args)
    p = load_profile()
    for currentVoxelSize in input_to_range(VoxelSize, p.VoxelSize):
        profile_environment(profile, currentVoxelSize, 1000, 3)
        create_config_impl(p)
        modify_config(profile, currentVoxelSize, Steps, Cycles, 1000, 3)

@task
def hemelb_profile(profile, VoxelSize=None, Steps=None, Cycles=None, create_configs=True, **args):
    """Submit HemeLB job(s) to the remote queue.
    The HemeLB config file(s) will optionally be prepared according to the profile and profile arguments given.
    This can submit a massive number of jobs -- do not use on systems with a limit to number of queued jobs permitted.
    """
    profile_environment(profile, VoxelSize, Steps, Cycles, args)
    p = load_profile()
    for currentVoxelSize in input_to_range(VoxelSize, p.VoxelSize):
        #Steps and cycles don't effect the geometry, so we can create these configs by copy-and-edit
        profile_environment(profile, currentVoxelSize, 1000, 3)
        if not str(create_configs).lower()[0] == 'f':
            pass
            create_config_impl(p)
        for currentSteps in input_to_range(Steps, 1000):
            for currentCycles in input_to_range(Cycles, 3):
                with_template_config()
                profile_environment(profile, currentVoxelSize, currentSteps, currentCycles)
                if not str(create_configs).lower()[0] == 'f':
                    modify_config(profile, currentVoxelSize, currentSteps, currentCycles, 1000, 3)
                execute(hemelbs, env.config, **args)

@task
def get_running_location(job=None):
    if job:
        with_job(job)
    env.running_node = run(template("cat $job_results/env_details.asc"))

def manual(cmd):
    #From the fabric wiki, bypass fabric internal ssh control
    commands = env.command_prefixes[:]
    if env.get('cwd'):
        commands.append("cd %s" % env.cwd)
    commands.append(cmd)
    manual_command = " && ".join(commands)
    pre_cmd = "ssh -Y -p %(port)s %(user)s@%(host)s " % env
    local(pre_cmd + "'" + manual_command + "'", capture=False)
    
def run(cmd):
    if env.manual_ssh:
        return manual(cmd)
    else:
        return fabric.api.run(cmd)
        
def put(src, dest):
    if env.manual_ssh:
        env.manual_src = src
        env.manual_dest = dest
        local(template("scp $manual_src $user@$host:$manual_dest"))
    else:
        fabric.api.put(src, dest)
        
@task
def vampir(original_job, *args):
    env.original_job = original_job
    env.original_job_results = env.pather.join(env.results_path, original_job)
    job(dict(job_name_template='vampir_${original_job}', script='vampir',
            cores=16, wall_time='0:15:0'), args)

@task
def vampir_tunnel(node, port):
    local("ssh hector -L 30070:nid%s:%s -N" % node, port)

@task
def steer(job, orbit=False, view=False, retry=False, framerate=None):
    with_job(job)
    if view:
        env.steering_client = 'steering.py'
    else:
        env.steering_client = 'timing_client.py'
    if orbit:
        env.steering_options = "--orbit"
    else:
        env.steering_options = ""
    if retry:
       env.steering_options += " --retry"
    if framerate:
        env.steering_options += " --MaxFramerate=%s" % framerate
    command_template = "python $repository_path/Tools/steering/python/hemelb_steering/${steering_client} ${steering_options} ${running_node} >> $job_results/steering_results.txt"       
    if retry:
        while True:
            try:
                get_running_location()
                run(template(command_template))
                break
            except:
                print "Couldn't connect. Will retry"
                execute(stat)
                time.sleep(10)
    else:
        get_running_location()
        run(template(command_template))


@task
def ensemble(config,cores,wall_time):
    current_directory=os.path.dirname(os.path.realpath(__file__))
    pyNS_dir=current_directory+"/pyNS-master"
    os.chdir(pyNS_dir)
    os.system("python Ensemble.py "+env.machine_name+ " "+cores+" "+config+" "+wall_time)




@task
def run_pyNS(config):
    current_directory=os.path.dirname(os.path.realpath(__file__))
    pyNS_dir=current_directory+"/pyNS-master"
    os.chdir(pyNS_dir)
    os.system("python pyNS-profiles.py "+config)



@task
def generate_LB(config):
    current_directory=os.path.dirname(os.path.realpath(__file__))
    pyNS_dir=current_directory+"/pyNS-master"
    os.chdir(pyNS_dir)
    os.system("python LB-configs.py "+config)


@task
def submit_jobs(config,cores,wall_time):
    current_directory=os.path.dirname(os.path.realpath(__file__))
    pyNS_dir=current_directory+"/pyNS-master"
    os.chdir(pyNS_dir)
    os.system("python hemelb-jobs.py "+env.machine_name+ " "+cores+" "+config+" "+wall_time)


