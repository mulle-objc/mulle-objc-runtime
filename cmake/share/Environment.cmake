### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# Environment.cmake
#    The part of the build environment that every build needs, whether
#    mulle-sde is involved or not: the project identifiers, the target
#    triple and the platform specific library naming.
#
#    Everything SDK related lives in SDKEnvironment.cmake, which this file
#    includes when USE_DEPENDENCY_DIR is on. Nothing in this file itself
#    looks for a dependency directory.
#
# INPUT
#    PROJECT_NAME               source for the three PROJECT_*IDENTIFIERs
#    USE_DEPENDENCY_DIR         declared as an option by this file, see
#                               OPTIONS. When off, SDKEnvironment is not
#                               included at all, so no dependency or
#                               addiction directory is searched
#    CMAKE_SYSTEM_PROCESSOR     part of MULLE_TARGET_TRIPLE
#    CMAKE_SYSTEM_NAME          part of MULLE_TARGET_TRIPLE
#    CMAKE_C_COMPILER_ID        part of MULLE_TARGET_TRIPLE, with
#    CMAKE_CXX_COMPILER_ID      ... as the fallback
#    CMAKE_BUILD_TYPE           "Debug" enables the MSVC postfix
#    BUILD_SHARED_LIBS          picks the WIN32 import library suffixes
#    MULLE_TRACE_INCLUDE        trace inclusion of this file
#
# ENVIRONMENT
#    none. MULLE_VIRTUAL_ROOT and the SDK variables are read by
#    SDKEnvironment.cmake
#
# OPTIONS
#    USE_DEPENDENCY_DIR         ON   include SDKEnvironment, so the
#                                    dependency and addiction directories
#                                    are searched for headers and libraries.
#                                    Forced off for a cmake subproject,
#                                    which has no such directories and wants
#                                    none. A plain cmake build of this
#                                    project on its own can pass
#                                    -DUSE_DEPENDENCY_DIR=OFF
#
# OUTPUT
#    IS_CMAKE_SUBPROJECT        ON when pulled in with add_subdirectory()
#    PROJECT_IDENTIFIER         snake cased PROJECT_NAME
#    PROJECT_UPCASE_IDENTIFIER  upper cased variant
#    PROJECT_DOWNCASE_IDENTIFIER lower cased variant
#    MULLE_TARGET_TRIPLE        <processor>-<system>-<compiler id>. Computed
#                               here, not in SDKEnvironment, because it is
#                               useful without an SDK too
#    TARGET_TRIPLET_LOWER       lower cased variant
#    CMAKE_FIND_LIBRARY_SUFFIXES  WIN32 only
#    CMAKE_DEBUG_POSTFIX        "d", MSVC Debug builds only
#
#    See SDKEnvironment.cmake for MULLE_VIRTUAL_ROOT, DEPENDENCY_DIR,
#    ADDICTION_DIR,
#    MULLE_SDK_PATH, MULLE_SDK_SUBDIR, MULLE_SDK_DEPENDENCY_DIR,
#    ADDITIONAL_BIN_PATH and the three CMAKE_*_PATH search paths.
#
# INCLUDES
#    StringCase, SDKEnvironment (if USE_DEPENDENCY_DIR),
#    MultiPhase (OPTIONAL), EnvironmentAux (OPTIONAL)
#
# GUARD
#    __ENVIRONMENT__CMAKE__
#


if( NOT __ENVIRONMENT__CMAKE__)
   set( __ENVIRONMENT__CMAKE__ ON)

   ## USE mulle-sde -v craft -- -DMULLE_TRACE_INCLUDE=ON to trace cmake
   ##     inclusion
   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( StringCase)

   if( NOT PROJECT_IDENTIFIER)
      snakeCaseString( "${PROJECT_NAME}" PROJECT_IDENTIFIER)
   endif()
   if( NOT PROJECT_UPCASE_IDENTIFIER)
      string( TOUPPER "${PROJECT_IDENTIFIER}" PROJECT_UPCASE_IDENTIFIER)
   endif()
   if( NOT PROJECT_DOWNCASE_IDENTIFIER)
      string( TOLOWER "${PROJECT_IDENTIFIER}" PROJECT_DOWNCASE_IDENTIFIER)
   endif()

   #
   # Are we the top level project, or were we pulled into another cmake project
   # with add_subdirectory() ?
   #
   if( NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
      set( IS_CMAKE_SUBPROJECT ON)
   endif()

   #
   # A cmake subproject has no SDK and no dependency folder, and wants none:
   # its dependencies are cmake targets supplied by the parent project. So turn
   # the dependency directory off before the option below declares it. A normal
   # variable wins over option() (CMP0077), and -DUSE_DEPENDENCY_DIR=... on the
   # command line wins over both.
   #
   if( IS_CMAKE_SUBPROJECT)
      set( USE_DEPENDENCY_DIR OFF)
   endif()

   #
   # Search dependency/ and addiction/ for headers and libraries. This is how a
   # mulle-sde build finds its crafted dependencies. For a plain cmake build of
   # this project on its own, pass -DUSE_DEPENDENCY_DIR=OFF.
   #
   option( USE_DEPENDENCY_DIR
           "Search the dependency and addiction directories for headers and libraries"
           ON)

   message( STATUS "IS_CMAKE_SUBPROJECT=${IS_CMAKE_SUBPROJECT}")
   message( STATUS "USE_DEPENDENCY_DIR=${USE_DEPENDENCY_DIR}")


   #
   # Loop invariant, and useful without an SDK too, so it lives here
   # rather than in SDKEnvironment.cmake
   #
   # Get the target triplet
   # On Unix, we add this to the search path on linux, for addictions that
   # install debian style. For dependencies that misbehave, use dispensing,
   # to copy the file to the right place: mulle-sde dep mark <x> no-inplace
   #
   if( DEFINED CMAKE_C_COMPILER_ID)
      set( MULLE_TARGET_TRIPLE "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_C_COMPILER_ID}")
   else()
      set( MULLE_TARGET_TRIPLE "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}")
   endif()
   string( TOLOWER "${MULLE_TARGET_TRIPLE}" TARGET_TRIPLET_LOWER)

   #
   # Everything SDK related lives in SDKEnvironment.cmake. USE_DEPENDENCY_DIR
   # off keeps it out of the build entirely. A cmake subproject pulled in
   # with add_subdirectory() always turns it off above: it has no dependency
   # directory and wants none.
   #
   if( USE_DEPENDENCY_DIR)
      include( SDKEnvironment)
   endif()

   #
   # add "d" to library names on windows MSVC for debugging libraries
   #
   if( MSVC AND NOT CMAKE_DEBUG_POSTFIX AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
      set( CMAKE_DEBUG_POSTFIX "d")
   endif()

   if( WIN32)
      if( BUILD_SHARED_LIBS)
         set( CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a;.a;.lib;.dll")
      else()
         set( CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib;.dll.a;.dll")
      endif()
   endif()


   include( MultiPhase OPTIONAL)

   include( EnvironmentAux OPTIONAL)

endif()
