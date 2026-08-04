# Working with multiple mulle projects
<!-- Keywords: mulle-sde, dependency, multi-project, multiple, projects, cache -->

Each project has separate ephemeral directories:

- `stash/`
- `kitchen/`
- `dependency/`

The `test/` directory is its own mulle-sde project with separate caches.

The `demo/` directory is also its own mulle-sde project with separate caches,
but likely will have sources from the main project referenced directly and
does NOT link the main project.

## Workflow

It is **assumed** `mulle-sde vibecoding on` has properly run, only then
is the following chart valid:

| Compile            | Changes In           | Command
|--------------------|----------------------|-----------------------------------------
| main               | main project only    | mulle-sde craft
| main               | dependency           | mulle-sde recraft
| test               | main project only    | mulle-sde test craft
| test               | dependency           | mulle-sde test recraft
| test               | test                 | mulle-sde test run
| demo               | main project or demo | mulle-sde -d demo craft
| demo               | dependency           | mulle-sde -d demo recraft
| demo               | main project or demo | mulle-sde -d demo craft


## Diagnostics

Redirect build output to logs and grep the log files instead of repeatedly
rerunning builds.
