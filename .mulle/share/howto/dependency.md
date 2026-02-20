# mulle-sde Dependency and File inclusion guidelines
<!-- Keywords: craft, build, run -->

You should add remote single files like `stb_image.h` or complete repositories
like  `zlib` with the `mulle-sde dependency` command. Check out the help
file for examples:

```bash
mulle-sde dependency help
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
│   ├── bin
│   ├── include
│   │   ├── include.h
│   │   ├── mulle-c11
│   │   │   ├── mulle-c11-align.h
│   │   │   └── mulle-c11-integer.h
│   │   └── mulle-time
│   │       ├── cmake
│   │       │   ├── DependenciesAndLibraries.cmake
│   │       │   ├── _Dependencies.cmake
│   │       │   └── _Libraries.cmake
│   │       ├── include.h
│   │       ├── mulle-timetype.h
│   │       └── mulle-timeval.h
│   ├── lib
│   │   ├── cmake
│   │   │   └── mulle-c11
│   │   │       └── mulle-c11-config.cmake
│   │   └── libmulle-time.so
│   └── share
│       └── mulle-time
│           └── dox
│               └── TOC.md
├── etc
│   ├── craftorder
│   ├── done--Default-linux-Debug
│   ├── done--Default-windows-Debug
│   ├── link--Default-linux-Debug
│   ├── link--Default-linux-Debug--startup
│   ├── link--Default-windows-Debug
│   └── link--Default-windows-Debug--startup
└── windows
    └── Debug
        ├── bin
        ├── include
        │   ├── include.h
        │   ├── mulle-c11
        │   │   ├── mulle-c11-align.h
        │   │   └── mulle-c11-integer.h
        │   └── mulle-time
        │       ├── cmake
        │       │   ├── DependenciesAndLibraries.cmake
        │       │   ├── _Dependencies.cmake
        │       │   └── _Libraries.cmake
        │       ├── include.h
        │       ├── mulle-absolutetime.h
        │       └── mulle-timeval.h
        ├── lib
        │   ├── cmake
        │   │   └── mulle-c11
        │   │       └── mulle-c11-config.cmake
        │   ├── libmulle-time.dll
        │   └── libmulle-time.dll.a
        └── share
            └── mulle-time
                └── dox
                    └── TOC.md
```

### /etc

```
├── etc
│   ├── craftorder
│   ├── done--Default-linux-Debug
│   ├── done--Default-windows-Debug
│   ├── link--Default-linux-Debug
│   ├── link--Default-linux-Debug--startup
│   ├── link--Default-windows-Debug
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
        ├── bin
        ├── include
        │   ├── include.h
        │   ├── mulle-c11
        │   │   ├── mulle-c11-align.h
```

### Cmake inheritance

```
├── Debug
│   ├── bin
│   ├── include
│   │   └── mulle-time
│   │       ├── cmake
│   │       │   ├── DependenciesAndLibraries.cmake
│   │       │   ├── _Dependencies.cmake
│   │       │   └── _Libraries.cmake
```

`DependenciesAndLibraries.cmake` is read by the custom mulle-sde cmake file
to inherit dependencies recursively from other dependencies.

### API


```
│   └── share
│       └── mulle-time
│           └── dox
│               └── TOC.md
```

These are the API files you can read with `mulle-sde api cat`.
