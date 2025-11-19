# mulle-sde Build System Guidelines
<!-- Keywords: craft, build, run -->

mulle-sde is an AI friendly build system, made to run on all platforms, that
run bash (i.e. macos, android, linux, windows, sunos, freebs). Because of that
complexity it manages the build process from beginning to end.

mulle-sde enforces a particular project structure. It also manages dependencies
and libraries. Do not attempt to run `cmake` or other build tools on your
own, that's a road into misery. Always use `mulle-sde` commands. Also try to
avoid editing ANY build system files like `Makefile` or `CMakeLists.txt`
generally mulle-sde will manage them for you. Avoid the inclusion of header
dependencies, again mulle-sde will manage this.

### Sources and Headers

-- Do not add sources and headers to build system files. Call `mulle-sde reflect` and it will all be done for you. You wille need to manually include your new headers into other project files of the same project though

### Dependencies and headers

- Do not include or import dependency headers: Exceptions are project local headers and C standard headers. The build system will handle the rest. Trust the build system.
- Add dependencies with the `mulle-sde dependency` command. Look for a dependency HOWTO.
- Add libraries with the `mulle-sde library` command. Look for a library HOWTO.

### 1.1 Standard Reflect


```bash
mulle-sde reflect
```

Updates the build system with new and deleted files.

### 1.1 Standard Build

To compile the project:

```bash
mulle-sde craft # -- --no-hook
```

Use --no-hook if you notice that your library project is part of an
amalgamation and you just want to look for errors.


### 1.2 Clean Build

For a full, clean build, including reinstalling dependencies:

```bash
mulle-sde craft -g # -- --no-hook
```

*(`-g` is shorthand for `mulle-sde clean gravetidy`)*

