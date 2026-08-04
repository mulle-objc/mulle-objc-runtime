# Troubleshooting Transitive Dependency Propagation
<!-- Keywords: mulle-sde, dependency, transitive, propagation, library, link, missing, cmake, inherit, DependenciesAndLibraries, no-cmake-inherit, downstream, indirect -->

## How library propagation works

When project A declares dependency X, and downstream project D depends on A,
X is automatically linked into D via cmake include files:

1. `mulle-sde reflect` generates `cmake/reflect/_Dependencies.cmake` listing
   A's dependencies with `find_library()` calls
2. `mulle-sde craft` installs these into
   `dependency/<config>/include/<A>/cmake/`
3. When D builds, it finds library A and includes
   `include/<A>/cmake/DependenciesAndLibraries.cmake` which recursively pulls
   in A's dependencies (and their dependencies, etc.)

## Symptom: library X not found when building downstream project D

D depends on A, A depends on X, but D fails with "X_LIBRARY was not found".

### Step 1: Check that X is not marked no-cmake-inherit in A

```bash
mulle-sde -d <A> dependency list -- --output-no-column --output-no-header --format "%a;%m\n" | grep <X>
```

If marks contain `no-cmake-inherit`, propagation is intentionally stopped.
Remove the mark if propagation is desired:

```bash
mulle-sde -d <A> dependency unmark <X> no-cmake-inherit
mulle-sde -d <A> reflect
mulle-sde -d <A> craft
```

### Step 2: Check that A installed its cmake files

```bash
ls dependency/<config>/include/<A>/cmake/
```

Should contain: `DependenciesAndLibraries.cmake`, `_Dependencies.cmake`,
`_Libraries.cmake`. If missing, A's CMakeLists.txt may lack the install rule:

```cmake
install(FILES ${INSTALL_CMAKE_INCLUDES} DESTINATION "include/${PROJECT_NAME}/cmake")
```

### Step 3: Check that X appears in A's installed _Dependencies.cmake

```bash
cat dependency/<config>/include/<A>/cmake/_Dependencies.cmake | grep -i <X>
```

If X is absent, run `mulle-sde -d <A> reflect` and recraft A.

### Step 4: Verify the cmake include path resolves

Enable cmake trace to see what's being included:

```bash
mulle-sde -DMULLE_TRACE_INCLUDE=ON craft
```

This prints each included cmake file. Check that A's
`DependenciesAndLibraries.cmake` is reached and that it finds X's cmake dir.

## Marks that control propagation

| Mark | Effect |
|------|--------|
| `no-link` | Dep skipped entirely in cmake output |
| `no-cmake-inherit` | Don't include dep's DependenciesAndLibraries.cmake (stops full chain) |
| `no-cmake-dependency` | Don't include DependenciesAndLibraries.cmake (subset of inherit) |
| `no-cmake-add` | Don't add to link list, but still inherit transitive deps |
| `no-cmake-searchpath` | Don't add dep's cmake dir to CMAKE_MODULE_PATH |
| `no-cmake-loader` | Don't look for runtime loader header |

## Quick verification

```bash
# show all transitive deps and their marks
mulle-sourcetree --recurse list -r --output-no-column --format "%a;%m\n"

# check what gets linked (after craft)
cat dependency/Debug/lib/Debug/*.a 2>/dev/null | wc -c  # libs exist?
cat cmake/reflect/_Dependencies.cmake | grep find_library  # what we declare
```
