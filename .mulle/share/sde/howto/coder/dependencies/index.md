# mulle-sde Dependency and File Inclusion Guidelines
<!-- Keywords: mulle-sde, sourcetree, dependency, craft, build, run -->

Use `mulle-sde dependency add` for third-party dependencies and your own
projects, and `mulle-sde library add` for system libraries.

## Basic commands

```bash
mulle-sde dependency help
mulle-sde library help
mulle-sde dependency insert github:name/repo
```

## Local project via search path

1. ensure the repo is reachable via `MULLE_FETCH_SEARCH_PATH`
2. add it as a normal github dependency
3. fetch/craft to get it symlinked into the stash

## Important

- dependency order matters
- keep startup and allocator-related support dependencies at the bottom
- use `mulle-sourcetree marks --show` to understand marks
