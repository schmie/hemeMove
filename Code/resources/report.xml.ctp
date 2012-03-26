<?xml version="1.0"?>
<report>
	<configuration>
		<file>{{CONFIG}}</file>
		<steps>{{TOTAL_TIME_STEPS}}</steps>
		<resolution>
			<timestep>{{TIME_STEP_LENGTH}}</timestep>
		</resolution>
	</configuration>
	{{#BUILD}}
	<build>
		<revision>{{REVISION}}</revision>
		<steering>{{STEERING}}</steering>
		<streaklines>{{STREAKLINES}}</streaklines>
		<multimachine>{{MULTIMACHINE}}</multimachine>
		<type>{{TYPE}}</type>
		<optimisation>{{OPTIMISATION}}</optimisation>
		<date>{{TIME}}</date>
		<reading_group>{{READING_GROUP_SIZE}}</reading_group>
		<lattice_type>{{LATTICE_TYPE}}</lattice_type>
		<kernel_type>{{KERNEL_TYPE}}</kernel_type>
		<wall_boundary_condition>{{WALL_BOUNDARY_CONDITION}}</wall_boundary_condition>
	</build>
	{{/BUILD}}
	<nodes>
		<threads>{{THREADS}}</threads>
		<machines>{{MACHINES}}</machines>
		<depths>{{DEPTHS}}</depths>
	</nodes>
	<geometry>
		<sites>{{SITES}}</sites>
		<blocks>{{BLOCKS}}</blocks>
		<sites_per_block>{{SITESPERBLOCK}}</sites_per_block>
		{{#PROCESSOR}}
		<domain>
			<rank>{{RANK}}</rank><sites>{{SITES}}</sites>
		</domain>
		{{/PROCESSOR}}
	</geometry>
	<results>
		<images>{{IMAGES}}</images>
		<snapshots>{{SNAPSHOTS}}</snapshots>
		<steps>
			<total>{{STEPS}}</total>
		</steps>
	</results>
	<checks>
		{{#DENSITIES}}
		<density_problem>
			<allowed>{{ALLOWED}}</allowed>
			<actual>{{ACTUAL}}</actual>
		</density_problem>
		{{/DENSITIES}}
		{{#UNSTABLE}}
			<stability_problem/>
		{{/UNSTABLE}}
	</checks>
	<timings>
		{{#TIMER}}
		<timer>
			<name>{{NAME}}</name>
			<local>{{LOCAL}}</local>
			<min>{{MIN}}</min>
			<mean>{{MEAN}}</mean>
			<max>{{MAX}}</max>
		</timer>
		{{/TIMER}}
	</timings>
</report>