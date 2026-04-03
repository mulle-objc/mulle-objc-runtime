# CMake Testing in mulle-sde
<!-- Keywords: cmake, test, add_subdirectory, external, consumer -->

Testing CMake-based projects and external consumer scenarios.

### Test Structure

```
test/XX-name/
├── CMakeLists.txt     # Test CMake configuration
├── test_file.c        # Test source
├── XX-name.exe        # Produced test file from cmake (always XX-name.exe)
└── default.stdout     # Expected output
```

## Regular Tests

- Use `include.h` for dependencies (auto-generated)
- Link with `TEST_LIBRARIES` variable (passed by mulle-sde test)
- Standard mulle-test workflow

### Key Variables

**Passed by mulle-sde test:**

- `TEST_LIBRARIES`: Libraries to link against (includes dependencies + rpath flags)
- `CMAKE_C_COMPILER`: C compiler to use
- `CMAKE_C_FLAGS`: C compiler flags (includes sanitizers, includes, defines)
- `CMAKE_CXX_FLAGS`: C++ compiler flags (same as C flags)
- `CMAKE_EXE_LINKER_FLAGS`: Executable linker flags (rpath + sanitizers)
- `CMAKE_SHARED_LINKER_FLAGS`: Shared library linker flags
- `CMAKE_RULE_MESSAGES`: Set to OFF (reduces build noise)

**Environment variables:**

- `DEPENDENCY_DIR`: Dependency headers/libs location
- `ADDICTION_DIR`: Additional dependencies
- `CC`/`CXX`: Compiler executables (shell environment, not CMake variables)


## External Consumer CMakeLists.txt Tests

- Test CMake target properties and configuration
- Verify build system behavior

This is a special case, where we want to verify git submodule like
behaviour external consumer scenarios (add_subdirectory)


```cmake
cmake_minimum_required(VERSION 3.19)
project(test_external_consumer)

# Test add_subdirectory behavior
add_subdirectory(../.. build-dir)

# Verify target properties
if(TARGET project-name)
    get_target_property(PROP project-name INTERFACE_LINK_OPTIONS)
    # Test the property value
endif()

# Create test executable
add_executable(test_exe test_file.c)
target_link_libraries(test_exe project-name)
```

### Common Patterns

- Path from test dir to project root: `../..`
- Binary directory required for out-of-tree sources
- Target name matches project name from `project()` declaration
- Check properties after `add_subdirectory()` call

