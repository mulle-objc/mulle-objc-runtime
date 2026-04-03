# mulle-sde Dependency and File inclusion guidelines
<!-- Keywords: mulle-sde, sourcetree, dependency, craft, build, run -->

You should add third party dependencies and your own projects that are missing
from project with `mulle-sde dependency add`.

You should add system libraries like `m` or `pthreads` with `mulle-sde library add`.

You should add remote single files like `stb_image.h` or complete repositories
like  `zlib` with the `mulle-sde dependency add` command.

Check out the help for bot commands for easy to reuse examples:

```bash
mulle-sde dependency help
mulle-sde library help
```

## Preference for github repositories

Use this style:

``` bash
mulle-sde dependency add github:name/repo
```

This will do what you need in most cases.
If you need to tweak the dependency read the documentation with regards to
"marks" of a sourcetree.

## Dependency Marks


To work with dependencies, you should be familiar with the sourcetree marks
that accompany each dependendy, they steer mulle-sde:

``` bash
mulle-sourcetree marks --show
mulle-sourcetree-to-c --show-marks
mulle-sourcetree-to-cmake --show-marks
```

Some important marks:

| Mark               | Meaning
|--------------------|----------------------------
|no-bequeath         | is not inheritable
|no-platform-windows | not available on windows

## Dependency Order

If dependency `a` depends on dependency `b` it must be ordered after `b`.
This rule is relaxed if both dependencies are `no-singlephase` and aren't
gapped by a dependency that is `singlephase` (as no `no-singlephase`).


## Debug cmake find_library

``` bash
mulle-sde -DCMAKE_DEBUG_FLAGS=--debug-find recraft
```

Check the logs for output or use `-v recraft`


## Understanding dependency directory organization

You can get the location with `mulle-sde dependency-dir`. The organization
is quite like the standard unix layout, so you should feel right at home.

The dependency directory contains per-platform, per-configuration and
per-sdk headers and libraries. The default `Release` configuration and the
default SDK `Default` are folded in.

```
/home/nat/.mulle/var/cache/sde/mulle-time-9ffd13004743/dependency
├── bin
├── Debug
├── etc
└── windows
```

### /etc

```
├── etc
│   ├── craftorder
|   |..
│   ├── done--Default-windows-Debug
|   |..
│   └── link--Default-windows-Debug--startup
```

The root etc folder contains important files for *crafting*. The `craftorder`
file is the list of dependencies to be built (depending on marks, contained
in the file). The `done-*` files specify on a per-SDK, per-platform,
per-configuration basis what has already been crafted.

The `link--*` files (found in test project only) contain the linker command
line arguments to use to link the dependencies.


### Headers

`<mulle-c11/mulle-c11-align.h>` for the *windows* platform in configuration
*Debug* resides here:

```
└── windows
    └── Debug
        |.. |..
        ├── include
        │   ├── include.h
        │   ├── mulle-c11
        │   │   ├── mulle-c11-align.h
        |   |..
```

### Cmake inheritance

```
├── Debug
|.. |..
│   ├── include
│   │   └── mulle-time
│   │       ├── cmake
│   │       │   ├── DependenciesAndLibraries.cmake
│   │       │   ├── _Dependencies.cmake
│   │       │   └── _Libraries.cmake
|   |       |..
```

`DependenciesAndLibraries.cmake` is read by the custom mulle-sde cmake file
to inherit dependencies recursively from other dependencies.

### API


```
├── Debug
|.. |..
│   └── share
│       └── mulle-time
│           └── dox
│               └── TOC.md
```

These are the API files you can read with `mulle-sde api cat`.


## Howto manage cross-platform library or dependency

A typical problem you need zlib on Linux as a system library but on Windows
you need to build from source.

### 1. Add system library

``` bash
mulle-sde library add --marks no-platform-windows z
```
### 2. Add dependency (source fallback)

``` bash
mulle-sde dependency add --marks only-platform-windows --github madler/zlib
mulle-sde dependency move zlib to top  # Must build before your project
```

### 3. Set library search aliases if needed

``` bash
# Check what the built library is actually called:
find $(mulle-sde dependency-dir)/windows/Debug/lib -name "*z*"
# Found: libzd.dll.a (debug), libzsd.a (static debug)

mulle-sde dependency set zlib aliases z,zs
```

### 4. Don't generate includes if code already has `#include <zlib.h>`

``` bash
mulle-sde library mark z no-header
mulle-sde dependency mark zlib no-header
```

