# mulle-sde Advanced Dependency Management
<!-- Keywords: mulle-sde, dependency, addiction, tarball, archive, preload, craft -->

This document covers advanced dependency handling topics not covered in the
basic dependency documentation, including tar archives, addictions, and
esoteric dependency configurations.

## Installing Tar Archives into Dependencies

You can preload the dependency folder with tarball content or directory
contents using the `DEPENDENCY_TARBALLS` environment variable.

### Basic Tarball Installation

Set the `DEPENDENCY_TARBALLS` environment variable to a colon-separated list
of tarball paths:

``` bash
export DEPENDENCY_TARBALLS="/path/to/library.tar.gz:/path/to/another.tgz"
mulle-sde craft
```

The tarballs will be automatically extracted into the dependency folder during
the craft process.

### Installation Location

Tarballs are extracted into:

```
dependency/Release/
```

For other configurations and platforms, they go into the appropriate
subdirectories based on the dispense style (see `mulle-craft style`).

### Framework Tarballs

Tarballs containing macOS/iOS frameworks are automatically detected and placed
into the `Frameworks` subdirectory if their filename contains:

- `.framework.` (case-insensitive)
- `.frameworks.` (case-insensitive)

Example:

``` bash
export DEPENDENCY_TARBALLS="MyLib.framework.tar.gz"
# Extracts into: dependency/Release/Frameworks/
```

### Directory Installation

You can also specify directories instead of tarballs. The directory contents
will be copied into the dependency folder:

``` bash
export DEPENDENCY_TARBALLS="/path/to/prebuilt:/path/to/archive.tar.gz"
```

### Tarball Structure

Your tarball should typically contain a standard Unix layout:

```
archive.tar.gz
├── include/
│   └── mylib.h
├── lib/
│   ├── libmylib.a
│   └── libmylib.so
└── bin/
    └── mylib-tool
```

This matches the dependency directory structure mulle-sde expects.

### Legacy Variable

The older `DEPENDENCY_TARBALL_PATH` environment variable is still supported
but deprecated. Use `DEPENDENCY_TARBALLS` instead.

### Use Cases

Tarball installation is useful for:

- Prebuilt proprietary libraries
- Binary distributions without source
- Cached build artifacts
- Platform-specific precompiled dependencies
- Speeding up CI/CD by avoiding rebuilds


## Addictions

Addictions provide an additional search path layer for headers, libraries,
frameworks, and binaries that supplements the dependency folder.

### What are Addictions?

Addictions are similar to dependencies but serve a different purpose:

- **Dependencies**: Built and managed by mulle-craft from source
- **Addictions**: External, preinstalled libraries/headers available system-wide

Think of addictions as a way to provide additional libraries that:

- Are already installed elsewhere on your system
- Should not be rebuilt by mulle-craft
- Need to be available during the build process
- Override or supplement dependency content

### Setting the Addiction Directory

Use the `ADDICTION_DIR` environment variable:

``` bash
export ADDICTION_DIR="/opt/custom-libs"
mulle-sde craft
```

Or use the mulle-craft built-in default:

``` bash
# Default location (if not overridden)
addiction/
```

The addiction directory uses the same structure as dependencies:

```
addiction/
├── include/          # Headers
├── lib/              # Libraries
├── bin/              # Executables
├── Frameworks/       # macOS Frameworks
└── etc/              # Craft metadata
```

### Platform-Specific Addictions

Addictions support platform and SDK-specific subdirectories:

```
addiction/
├── Default-linux-x86_64/
│   ├── include/
│   └── lib/
├── Default-darwin/
│   ├── include/
│   └── Frameworks/
├── include/          # Fallback for all platforms
└── lib/              # Fallback for all platforms
```

### Search Path Priority

The search path order is:

1. Dependency folder (platform-specific)
2. Dependency folder (fallback)
3. **Addiction folder (platform-specific)**
4. **Addiction folder (fallback)**

This means **dependencies override addictions** if both provide the same file.

### When to Use Addictions

Use addictions when you:

- Have preinstalled vendor SDKs (e.g., GPU libraries, platform SDKs)
- Share common libraries across multiple projects
- Want to provide platform-specific tools/libraries
- Need libraries that can't be built with mulle-craft
- Have proprietary binary-only libraries

### Disabling Addiction Search

You can exclude addiction paths when needed:

``` bash
mulle-craft searchpath --no-addiction header
```

Or only use addictions (no dependencies):

``` bash
mulle-craft searchpath --only-addiction library
```

### Addiction vs Dependency Directory Rules

**Important**: Do not nest these directories:

``` bash
# ❌ WRONG - Will fail
DEPENDENCY_DIR="/project/dependency"
ADDICTION_DIR="/project/dependency/addiction"

# ✅ CORRECT - Separate locations
DEPENDENCY_DIR="/project/dependency"
ADDICTION_DIR="/project/addiction"
```

Also avoid misleading names:

``` bash
# ❌ WRONG - Directory named "addiction" used as dependency
DEPENDENCY_DIR="/project/addiction"

# ✅ CORRECT - Clear naming
DEPENDENCY_DIR="/project/dependency"
ADDICTION_DIR="/project/addiction"
```


## Dependency Directory Structure Deep Dive

Understanding the internal structure helps with manual installations and
debugging.

### Root Level Organization

```
dependency/
├── bin/              # Default binaries (Release, Default SDK)
├── include/          # Default headers (Release, Default SDK)
├── lib/              # Default libraries (Release, Default SDK)
├── Frameworks/       # Default frameworks (Release, Default SDK)
├── etc/              # Craft metadata
├── Debug/            # Debug configuration
├── windows/          # Windows platform
└── darwin/           # macOS platform
```

### The etc/ Directory

Contains critical craft metadata:

```
dependency/etc/
├── craftorder                    # List of dependencies to build
├── done--Default-linux-Debug     # Build completion markers
├── done--Default-windows-Release
└── link--Default-linux-Debug--startup  # Linker arguments (test projects)
```

**craftorder**: Lists all dependencies with their marks, determines build order

**done-\*** files: Track completed builds per SDK/platform/configuration

**link-\*** files: Generated linker command line arguments for tests

### Platform Subdirectories

For cross-platform projects:

```
dependency/windows/Debug/
├── include/
├── lib/
└── share/
    └── <dependency-name>/
        └── cmake/
            └── DependenciesAndLibraries.cmake
```

### Cmake Inheritance Files

CMake projects recursively inherit dependencies via:

```
dependency/Debug/include/<library>/cmake/
├── DependenciesAndLibraries.cmake   # Main file included by mulle-sde
├── _Dependencies.cmake              # Transitive dependencies
└── _Libraries.cmake                 # Required libraries
```

These are automatically generated during the craft process.

### API Documentation

```
dependency/Debug/share/<library>/dox/
└── TOC.md
```

Read with: `mulle-sde api cat <library>`


## Advanced Dependency Configurations

### Installing Non-Linkable Dependencies

Some dependencies provide headers or tools but no libraries:

``` bash
mulle-sde dependency add --github user/header-only-lib
mulle-sde dependency mark header-only-lib no-link
mulle-sde dependency mark header-only-lib no-header  # If already using #include directly
```

### Per-Platform Dependencies with Fallbacks

#### System library on Linux, build from source on Windows:

``` bash
# Add system library (not on Windows)
mulle-sde library add --marks no-platform-windows z

# Add source dependency (Windows only)
mulle-sde dependency add --marks only-platform-windows --github madler/zlib
mulle-sde dependency move zlib to top
```

#### Handle different library names:

``` bash
# Check actual library name after building
find $(mulle-sde dependency-dir)/windows/Debug/lib -name "*z*"

# Set aliases if needed (e.g., found: libzd.dll.a, libzsd.a)
mulle-sde dependency set zlib aliases z,zs,zd,zsd
```

### Configuration-Specific Dependencies

Exclude dependencies from certain configurations:

``` bash
# Only needed for Debug builds
mulle-sde dependency mark test-framework no-craft-configuration-Release

# Only needed for Release builds
mulle-sde dependency mark optimizer no-craft-configuration-Debug
```

### SDK and OS-Specific Dependencies

``` bash
# Only craft on specific OS (host OS, not target platform)
mulle-sde dependency mark macos-lib no-craft-os-linux

# Only craft for specific target platform
mulle-sde dependency mark linux-headers no-craft-platform-darwin

# Only craft for specific SDK
mulle-sde dependency mark ios-lib no-craft-sdk-Default
mulle-sde dependency mark ios-lib only-craft-sdk-iPhoneOS
```

### Optional Dependencies

Make dependencies optional (ignore if unavailable):

``` bash
# Generally optional
mulle-sde dependency mark optional-lib no-require

# Optional for specific configuration
mulle-sde dependency mark debug-tool no-require-configuration-Release

# Optional for specific platform
mulle-sde dependency mark windows-lib no-require-platform-darwin
```


## Debugging Dependency Issues

### View Dependency Directory Location

``` bash
mulle-sde dependency-dir
```

### Check Craftorder

``` bash
cat "$(mulle-sde dependency-dir)/etc/craftorder"
```

Shows all dependencies with their marks.

### Check Build Status

``` bash
mulle-craft status
```

Shows which dependencies have been built successfully.

### Inspect Done Files

``` bash
ls -la "$(mulle-sde dependency-dir)/etc/"
```

One `done--*` file per SDK/platform/configuration combination indicates
successful builds.

### Debug CMake Library Search

``` bash
mulle-sde -DCMAKE_DEBUG_FLAGS=--debug-find recraft -v
```

Shows CMake's library search process with verbose output.

### Check Search Paths

``` bash
# View header search paths
mulle-craft searchpath header

# View library search paths
mulle-craft searchpath library

# View framework search paths
mulle-craft searchpath framework

# Include addictions
mulle-craft searchpath --if-exists library
```

### Manual Dependency Installation

If you need to manually install into the dependency folder:

``` bash
# Get current directory
DEPDIR="$(mulle-sde dependency-dir)"

# For Release/Default
cp -R /path/to/headers/* "$DEPDIR/include/"
cp /path/to/lib*.a "$DEPDIR/lib/"

# For Debug configuration
cp -R /path/to/headers/* "$DEPDIR/Debug/include/"
cp /path/to/lib*_d.a "$DEPDIR/Debug/lib/"

# For Windows platform, Release
cp -R /path/to/headers/* "$DEPDIR/windows/include/"
cp /path/to/lib*.a "$DEPDIR/windows/lib/"
```


## Environment Variables Reference

| Variable                       | Purpose                                          |
|--------------------------------|--------------------------------------------------|
| `DEPENDENCY_DIR`               | Main dependency directory location               |
| `DEPENDENCY_TARBALLS`          | Colon-separated list of tarballs to preload      |
| `ADDICTION_DIR`                | Additional search path directory                 |
| `MULLE_CRAFT_DISPENSE_STYLE`   | How built products are organized (auto, strict)  |
| `MULLE_CRAFT_SDKS`             | SDKs to build for (space-separated)              |
| `MULLE_CRAFT_PLATFORMS`        | Platforms to build for (space-separated)         |
| `MULLE_CRAFT_CONFIGURATIONS`   | Configurations to build (Debug, Release, etc.)   |


## See Also

- [dependency.md](dependency.md) - Basic dependency management
- [dependency-examples.md](dependency-examples.md) - Dependency examples
- [craft.md](craft.md) - Building and crafting
- [crosscompiling.md](crosscompiling.md) - Cross-platform builds
