cmake_minimum_required( VERSION 3.13...99.99)

project( mulle-objc-runtime VERSION 0.26.0 LANGUAGES C)

# Add suppression flags based on compiler
if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
  # GCC and Clang suppression flags
  add_compile_options(
    -Wno-unknown-pragmas
  )
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
  # MSVC suppression flags
  add_compile_options(
    /wd4068  # Unknown pragma warning
  )
  # MSVC secure function warnings suppression
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/reflect")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include( Environment)

include( CCompiler)

include( Files)


### Library

if( HEADERS_PHASE)
   install( FILES ${INSTALL_PUBLIC_HEADERS}  DESTINATION "include/${PROJECT_NAME}")
   install( FILES ${INSTALL_PRIVATE_HEADERS} DESTINATION "include/${PROJECT_NAME}")
   install( FILES ${INSTALL_CMAKE_INCLUDES}  DESTINATION "include/${PROJECT_NAME}/cmake")

   # short cut out
   if( NOT COMPILE_PHASE AND NOT LINK_PHASE)
      return()
   endif()
endif()

if( LINK_PHASE)
   include( Dependencies OPTIONAL)
endif()

include( Library)

message( STATUS "CMAKE_VERSION=${CMAKE_VERSION}")
# use LIBRARY_COMPILE_TARGET for compiler options
if( ${CMAKE_VERSION} VERSION_GREATER "3.14.0") 
   target_compile_definitions( "${LIBRARY_COMPILE_TARGET}" PUBLIC)
   if( NOT MSVC)
      target_compile_options( "${LIBRARY_COMPILE_TARGET}" PUBLIC -Wunused)
   endif()

   # use LIBRARY_LINK_TARGET for linker options
   if( LINK_PHASE)
      target_link_options( "${LIBRARY_LINK_TARGET}" PUBLIC)

      # Allow unresolved symbols on Windows (for mulle_atexit, __register_mulle_objc_universe)
      if( WIN32 OR MINGW)
         target_link_options( "${LIBRARY_LINK_TARGET}" PRIVATE "-Wl,/FORCE:UNRESOLVED")
      endif()
   endif()
endif()

include( InstallLibrary)

if( LINK_PHASE)
   include( InstallCMakeInclude)
   include( InstallCMakePackage)
endif()


if( APPLE)
   # Modern CMake handles lib path automatically
   set( CMAKE_SKIP_RPATH FALSE)
   if( CMAKE_VERSION VERSION_LESS 3.20)
      set(CMAKE_BUILD_RPATH
         "@loader_path/../lib/"
         "@loader_path/../Frameworks/"
      )
   endif()
endif()


# ugly hack
if( BUILD_SHARED_LIBS)
   remove_definitions("-DMULLE_INCLUDE_DYNAMIC=1")
endif()

#
# mulle-objc-uniqueid
#
set( EXECUTABLE_NAME mulle-objc-uniqueid)

# only need the hash function (done like this if runtime is built as shared)
set( EXECUTABLE_SOURCES  ${UNIQUEID_SOURCES}
src/mulle-objc-uniqueid.c
)

set( EXECUTABLE_DEPENDENCY_NAMES "")
# only need the hash function
set( EXECUTABLE_LIBRARY_LIST "${MULLE__CORE_LIBRARY}")
if( LIBBACKTRACE_LIBRARY)
   # needef when testing with .so
   list(APPEND EXECUTABLE_LIBRARY_LIST "${LIBBACKTRACE_LIBRARY}")
endif()

include( Executable)

#
# mulle-objc-signature
#
set( EXECUTABLE_NAME mulle-objc-signature)
# only need the signature function
set( EXECUTABLE_SOURCES  ${SIGNATURE_SOURCES}
src/mulle-objc-signature.c)
set( EXECUTABLE_LIBRARY_LIST "")
set( EXECUTABLE_DEPENDENCY_NAMES "")

include( Executable)

include( InstallExecutable)

include( FinalOutput OPTIONAL)
