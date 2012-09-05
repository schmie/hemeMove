# 
# Copyright (C) University College London, 2007-2012, all rights reserved.
# 
# This file is part of HemeLB and is CONFIDENTIAL. You may not work 
# with, install, use, duplicate, modify, redistribute or share this
# file, or any part thereof, other than as allowed by any agreement
# specifically made by you with University College London.
# 

"""
Module defining how we configure the fabric environment for target machines.
Environment is loaded from YAML dictionaries machines.yml and machines_user.yml
"""
# If we're running in an activated virtualenv, use that.
import site
from os import environ
from os.path import join
from sys import version_info
import sys
if 'VIRTUAL_ENV' in environ:
    virtual_env = join(environ.get('VIRTUAL_ENV'),
                       'lib',
                       'python%d.%d' % version_info[:2],
                       'site-packages')
    site.addsitedir(virtual_env)
    print 'Using Virtualenv =>', virtual_env
del site, environ, join, version_info
import fabric.api
from fabric.api import *
import os
import sys
import subprocess
import posixpath
import yaml
from templates import *
from functools import *
from pprint import PrettyPrinter
pp=PrettyPrinter()



#Root of local HemeLB checkout.
env.localroot=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
env.regression_tests_root=os.path.join(os.path.dirname(env.localroot),'RegressionTests')
env.no_ssh=False
env.no_hg=False
#Load and invoke the default non-machine specific config JSON dictionaries.
config=yaml.load(open(os.path.join(env.localroot,'deploy','machines.yml')))
env.update(config['default'])
user_config=yaml.load(open(os.path.join(env.localroot,'deploy','machines_user.yml')))
env.update(user_config['default'])
cmake_options=yaml.load(open(os.path.join(env.localroot,'deploy','compile_options.yml')))
env.verbose=False
env.needs_tarballs=False
env.pather=posixpath
env.remote=None
env.node_type=None
env.machine_name=None
env.node_type_restriction=""
env.node_type_restriction_template=""
# Maximum number of characters permitted in the name of a job in the queue system
# -1 for unlimited
env.max_job_name_chars=None

@task
def diagnostics():
    pp.pprint(env)
    pp.pprint(sys.path)
    import HemeLbSetupTool
    print HemeLbSetupTool.__path__

@task
def machine(name):
    """
    Load the machine-specific configurations.
    Completes additional paths and interpolates them, via complete_environment.
    Usage, e.g. fab machine:hector build
    """
    if "import" in config[name]:
        # Config for this machine is based on another
        env.update(config[config[name]["import"]])
        if config[name]["import"] in user_config:
            env.update(user_config[config[name]["import"]])
    env.update(config[name])
    if name in user_config:
        env.update(user_config[name])
    env.machine_name=name
    complete_environment()

#Metaprogram the machine wrappers
for machine_name in set(config.keys())-set(['default']):
    globals()[machine_name]=task(alias=machine_name)(partial(machine,machine_name))

def complete_environment():
    """Add paths to the environment based on information in the JSON configs.
    Templates are filled in from the dictionary to allow $foo interpolation in the JSON file.
    Environment vars created can be used in job-script templates:
    results_path: Path to store results
    remote_path: Root of area for checkout and build on remote
    config_path: Path to store config files
    repository_path: Path of remote mercurial checkout
    tools_path: Path of remote python 'Tools' folder
    tools_build_path: Path of disttools python 'build' folder for python tools
    regression_test_path: Path on remote to diffTest
    build_path: Path on remote to HemeLB cmake build area.
    install_path: Path on remote to HemeLB cmake install area.
    scripts_path: Path where job-queue-submission scripts generated by Fabric are sent.
    cmake_flags: Flags to pass to cmake
    run_prefix: Command string to invoke before any job is run.
    build_prefix: Command string to invoke before builds/installs are attempted
    build_number: Tip revision number of mercurial repository.
    build_cache: CMakeCache.txt file on remote, used to capture build flags.
    """
    env.hosts=['%s@%s'%(env.username,env.remote)]
    env.home_path=template(env.home_path_template)
    env.runtime_path=template(env.runtime_path_template)
    env.work_path=template(env.work_path_template)
    env.remote_path=template(env.remote_path_template)
    env.install_path=template(env.install_path_template)

    env.results_path=env.pather.join(env.work_path,"results")
    env.config_path=env.pather.join(env.work_path,"config_files")
    env.profiles_path=env.pather.join(env.work_path,"profiles")
    env.scripts_path=env.pather.join(env.work_path,"scripts")
    env.build_path=env.pather.join(env.remote_path,'build')
    env.code_build_path=env.pather.join(env.remote_path,'code_build')
    env.repository_path=env.pather.join(env.remote_path,env.repository)
    
    env.local_results=os.path.expanduser(template(env.local_results))
    env.local_configs=os.path.expanduser(template(env.local_configs))
    env.local_profiles=os.path.expanduser(template(env.local_profiles))
  
    env.local_templates_path=os.path.expanduser(template(env.local_templates_path))
  
    env.tools_path=env.pather.join(env.repository_path,"Tools")
    env.regression_test_repo_path=env.pather.join(env.pather.dirname(env.repository_path),"RegressionTests")
    env.regression_test_source_path=env.pather.join(env.regression_test_repo_path,"diffTest")
    env.regression_test_path=template(env.regression_test_path_template)
    env.tools_build_path=env.pather.join(env.install_path,env.python_build,'site-packages')
    module_commands=["module %s"%module for module in env.modules]
    env.build_prefix=" && ".join(module_commands+env.build_prefix_commands) or 'echo Building...'
    run_prefix_commands=env.run_prefix_commands[:]
    run_prefix_commands.append("export PYTHONPATH=$$PYTHONPATH:$tools_build_path")
    if env.temp_path_template:
        env.temp_path=template(env.temp_path_template)
        run_prefix_commands.append(template("export TMP=$temp_path"))
        run_prefix_commands.append(template("export TMPDIR=$temp_path"))

    env.run_prefix=" && ".join(module_commands+map(template,run_prefix_commands)) or 'echo Running...'
    #env.build_number=subprocess.check_output(['hg','id','-q'.'-i']).strip()
    # check_output is 2.7 python and later only. Revert to oldfashioned popen.
    cmd=os.popen(template("hg id -q -i"))
    env.build_number=cmd.read().strip()
    cmd.close()
    #env.build_number=run("hg id -q -i")
    env.build_cache=env.pather.join(env.build_path,'CMakeCache.txt')
    env.code_build_cache=env.pather.join(env.code_build_path,"CMakeCache.txt")

complete_environment()
