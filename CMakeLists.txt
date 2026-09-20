cmake_minimum_required( VERSION 3.13...99.99)

project( mulle-objc-runtime VERSION 0.30.0 LANGUAGES C)

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
  add_compile_definitions( _CRT_SECURE_NO_WARNINGS)
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
   # target_compile_definitions( "${LIBRARY_COMPILE_TARGET}" PUBLIC)
   if( NOT MSVC)
      target_compile_options( "${LIBRARY_COMPILE_TARGET}" PUBLIC -Wunused)
   endif()

   # use LIBRARY_LINK_TARGET for linker options
   if( LINK_PHASE)
      target_link_options( "${LIBRARY_LINK_TARGET}" PUBLIC)

      # Allow unresolved symbols on Windows (for mulle_atexit, __register_mulle_objc_universe)
      # if( WIN32 OR MINGW)
      #    target_link_options( "${LIBRARY_LINK_TARGET}" PRIVATE "-Wl,/FORCE:UNRESOLVED")
      # endif()
   endif()
endif()

include( InstallLibrary)

if( LINK_PHASE)
   include( AddSubdirectorySupport)
   include( FindPackageSupport)
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


# ugly hack - DISABLED: This prevents inline weak stubs from being created
# which causes undefined symbol errors on Windows
# if( BUILD_SHARED_LIBS)
#    remove_definitions("-DMULLE_INCLUDE_DYNAMIC=1")
# endif()

# The mulle-objc-uniqueid and mulle-objc-signature tools now live in the
# separate mulle-objc-tools project, which links this library.


include( FinalOutput OPTIONAL)
