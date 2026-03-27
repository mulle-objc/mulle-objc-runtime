# Toolchain file for cross-compiling to Windows via llvm-mingw + Clang

# The target system
set(CMAKE_SYSTEM_NAME Windows)
# You may change this to i686, aarch64, etc
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# And this too
set(TRIPLET x86_64-w64-mingw32)

# CRITICAL: Override system include flags BEFORE compiler detection
# CMake 3.24+ uses -external:I for Clang on Windows, but llvm-mingw doesn't support it
set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem " CACHE STRING "C system include flag" FORCE)
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem " CACHE STRING "CXX system include flag" FORCE)

# Ensure these are propagated to try_compile tests
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES 
     CMAKE_INCLUDE_SYSTEM_FLAG_C 
     CMAKE_INCLUDE_SYSTEM_FLAG_CXX
     CMAKE_C_COMPILER_FRONTEND_VARIANT
     CMAKE_CXX_COMPILER_FRONTEND_VARIANT)

# Force CMake to treat this as MinGW, not MSVC
# This must be set BEFORE any compiler detection
set(CMAKE_C_COMPILER_ID "Clang" CACHE STRING "C compiler ID" FORCE)
set(CMAKE_CXX_COMPILER_ID "Clang" CACHE STRING "CXX compiler ID" FORCE)
set(CMAKE_C_COMPILER_FRONTEND_VARIANT "GNU" CACHE STRING "C compiler frontend" FORCE)
set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT "GNU" CACHE STRING "CXX compiler frontend" FORCE)

# Identify as mulle-clang for Objective-C support (must match detection in CompilerDetectionC.cmake)
set(MULLE_C_COMPILER_ID "MULLECLANG")
set(MULLE_CXX_COMPILER_ID "MULLECLANG")

# Tell CMake where to find our platform override files
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Platform" ${CMAKE_MODULE_PATH})

# Root of your llvm-mingw installation
# Check for environment variable (set by mulle-sde platform system)
if(NOT DEFINED LLVM_MINGW_ROOT)
    if(DEFINED ENV{MULLE_CROSS_COMPILER_ROOT})
        set(LLVM_MINGW_ROOT $ENV{MULLE_CROSS_COMPILER_ROOT})
    elseif(DEFINED ENV{LLVM_MINGW_ROOT})
        set(LLVM_MINGW_ROOT $ENV{LLVM_MINGW_ROOT})
    else()
        # Fallback to default for now
        set(LLVM_MINGW_ROOT "/opt/mulle-clang-project-windows/latest")
        message(STATUS "Using default LLVM_MINGW_ROOT. Set with: mulle-sde platform set windows root /path")
    endif()
endif()

message(STATUS "Using LLVM-Mingw root: ${LLVM_MINGW_ROOT}")

# Target triplet

# Compilers - override any mulle-make defaults
set(CMAKE_C_COMPILER   ${LLVM_MINGW_ROOT}/bin/${TRIPLET}-clang CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER ${LLVM_MINGW_ROOT}/bin/${TRIPLET}-clang++ CACHE FILEPATH "CXX compiler" FORCE)
set(CMAKE_RC_COMPILER  ${LLVM_MINGW_ROOT}/bin/${TRIPLET}-windres CACHE FILEPATH "RC compiler" FORCE)

# Root path for finding target libs/includes
set(CMAKE_FIND_ROOT_PATH
    ${LLVM_MINGW_ROOT}/${TRIPLET}
    ${LLVM_MINGW_ROOT}/generic-w64-mingw32
    ${LLVM_MINGW_ROOT}
)


# Search modes: programs on host, libs/includes on target
# we also have programs in DEPENDENCY though, so we search both
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

set( BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries for Windows by default too")
set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" CACHE STRING "Build and search static libraries with .a")

# Prevent CMake from treating this as MSVC
set(MSVC FALSE)
set(CMAKE_COMPILER_IS_MINGW TRUE)
set(MINGW TRUE)

# Optional flags
set(CMAKE_CXX_FLAGS_INIT   "-fuse-ld=lld")
# Export all symbols so DLLs can find them via dlsym (needed for mulle-atinit)
#set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -Wl,--export-all-symbols")
