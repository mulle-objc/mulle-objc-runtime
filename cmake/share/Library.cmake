### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# Library.cmake
#    Creates the library target and links it. Sources are compiled in two
#    stages, so that PostLibrary can analyse the first stage and generate
#    sources for the second. A library without sources becomes an INTERFACE
#    library.
#
#    Everything that links happens inside LINK_PHASE only.
#
# INPUT
#    PROJECT_NAME              fallback for LIBRARY_NAME
#    LIBRARY_NAME              name of the library target
#    LIBRARY_SOURCES           defaults to SOURCES
#    LIBRARY_RESOURCES         defaults to RESOURCES
#    STAGE2_SOURCES            compiled after the first stage
#    STAGE2_HEADERS            headers for the second stage
#    OTHER_LIBRARY_OBJECT_FILES                       pre-compiled objects
#    OTHER_<LIBRARY_UPCASE_IDENTIFIER>_OBJECT_FILES   likewise
#    INSTALL_PUBLIC_HEADERS    added to the target so frameworks install them
#    INSTALL_PRIVATE_HEADERS   likewise
#    BUILD_SHARED_LIBS         static or shared
#    LINK_PHASE                gates all linking, from MultiPhase
#    DEPENDENCY_LIBRARIES      and the OPTIONAL_/STARTUP_/OS_SPECIFIC_ and
#                              ALL_LOAD_ variants, from Dependencies
#    HEADER_ONLY_LIBRARIES     from Dependencies. Entries that are cmake
#                              targets are linked for their include
#                              directories and build order only
#    MULLE_TRACE_INCLUDE       trace inclusion of this file
#
# OPTIONS
#    DLL_EXPORT_ALL             ON   sets CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
#    SHARED_UNRESOLVED_SYMBOLS  ON   shared libraries may have unresolved
#                                    symbols
#
#    See AddSubdirectorySupport.cmake for MULLE_TRANSITIVE_LINK_LIBRARIES
#    and MULLE_TRANSITIVE_SOURCE_INCLUDE, which together decide what an
#    add_subdirectory consumer inherits from this library
#
# OUTPUT
#    LIBRARY_IDENTIFIER            snake cased LIBRARY_NAME
#    LIBRARY_UPCASE_IDENTIFIER     upper cased variant
#    LIBRARY_DOWNCASE_IDENTIFIER   lower cased variant
#    LIBRARY_COMPILE_TARGET        target carrying compile options
#    LIBRARY_LINK_TARGET           target carrying link options
#    LIBRARY_STAGE2_TARGET         only when STAGE2_SOURCES exist
#    INSTALL_LIBRARY_TARGETS       appended, consumed by InstallLibrary
#    INSTALL_<LIBRARY_UPCASE_IDENTIFIER>_RESOURCES  consumed by InstallLibrary
#    CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS   from DLL_EXPORT_ALL
#
#    Creates the targets ${LIBRARY_NAME}, and _1_/_2_ prefixed helper
#    targets for the two compile stages.
#
# INCLUDES
#    StringCase, PreLibrary (OPTIONAL), Rpath (OPTIONAL),
#    LibraryAux (OPTIONAL)
#
# GUARD
#    none. Include once per library
#

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

### Library

if( NOT LIBRARY_NAME)
   set( LIBRARY_NAME "${PROJECT_NAME}")
endif()

include( StringCase)

if( NOT LIBRARY_IDENTIFIER)
   snakeCaseString( "${LIBRARY_NAME}" LIBRARY_IDENTIFIER)
endif()
if( NOT LIBRARY_UPCASE_IDENTIFIER)
   string( TOUPPER "${LIBRARY_IDENTIFIER}" LIBRARY_UPCASE_IDENTIFIER)
endif()
if( NOT LIBRARY_DOWNCASE_IDENTIFIER)
   string( TOLOWER "${LIBRARY_IDENTIFIER}" LIBRARY_DOWNCASE_IDENTIFIER)
endif()

if( BUILD_SHARED_LIBS)
   message( STATUS "Building dynamic \"${LIBRARY_NAME}\"")
else()
   message( STATUS "Building static \"${LIBRARY_NAME}\"")
endif()

# if( NOT LIBRARY_DOWNCASE_IDENTIFIER)
#    string( TOLOWER "${LIBRARY_IDENTIFIER}" LIBRARY_DOWNCASE_IDENTIFIER)
# endif()


if( NOT LIBRARY_SOURCES)
   set( LIBRARY_SOURCES "${SOURCES}")
   set( __LIBRARY_SOURCES_UNSET ON)
endif()

if( NOT LIBRARY_RESOURCES)
   set( LIBRARY_RESOURCES "${RESOURCES}")
   set( __LIBRARY_RESOURCES_UNSET ON)
endif()

option( DLL_EXPORT_ALL "Export all global symbols for DLL" ON)
set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ${DLL_EXPORT_ALL})


include( PreLibrary OPTIONAL)

# support header only library, and library just made up of pre-compiled
# object files
if( LIBRARY_SOURCES OR OTHER_LIBRARY_OBJECT_FILES OR OTHER_${LIBRARY_UPCASE_IDENTIFIER}_OBJECT_FILES)
   # RPATH must be ahead of add_library, but is it really needed ?
   include( Rpath OPTIONAL)


   set( ALL_OBJECT_FILES
      ${OTHER_LIBRARY_OBJECT_FILES}
      ${OTHER_${LIBRARY_UPCASE_IDENTIFIER}_OBJECT_FILES}
   )

   # Libraries are built in two stages:
   #
   # In the first step the PROJECT_FILES are compiled.
   # In the second step STAGE2_SOURCES are added.
   # This allows PostLibrary to run an analysis step over PROJECT_FILES and
   # generate files to be included by STAGE2_SOURCES. If there are no
   # STAGE2_SOURCES then this is just a more verbose way of doing it.
   # OBJC_DEPS_INC is the generated analysis step.
   #
   # This also enables parallel builds, when the products for a link aren't
   # available yet.
   #

   if( LIBRARY_SOURCES)
      set( LIBRARY_COMPILE_TARGET "_1_${LIBRARY_NAME}")
      set( LIBRARY_LINK_TARGET "${LIBRARY_NAME}")

      #
      # In the compile-only phase the library target does not exist yet (it
      # is created below under LINK_PHASE). Point the link target at the
      # compile target, so unguarded target_link_options() calls in project
      # CMakeLists.txt do not fail. An OBJECT library does not link, so the
      # options are simply ignored.
      #
      if( NOT LINK_PHASE)
         set( LIBRARY_LINK_TARGET "${LIBRARY_COMPILE_TARGET}")
      endif()

      add_library( ${LIBRARY_COMPILE_TARGET} OBJECT
         ${LIBRARY_SOURCES}
      )

      set( ALL_OBJECT_FILES
         $<TARGET_OBJECTS:${LIBRARY_COMPILE_TARGET}>
         ${ALL_OBJECT_FILES}
      )

      set_target_properties( ${LIBRARY_COMPILE_TARGET}
         PROPERTIES
            CXX_STANDARD 11
      )
      target_compile_definitions( ${LIBRARY_COMPILE_TARGET} PRIVATE "${LIBRARY_UPCASE_IDENTIFIER}_BUILD")
      if( BUILD_SHARED_LIBS)
         set_property( TARGET ${LIBRARY_COMPILE_TARGET} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
      endif()

      #
      # add_subdirectory support:
      #
      # An OBJECT library does not link, so it does not pick up the usage
      # requirements (include directories, definitions) of our dependencies
      # by itself. When a dependency resolved to a cmake target - which is
      # what happens when it was pulled in with add_subdirectory - link
      # against it. This gives us its INTERFACE_INCLUDE_DIRECTORIES, which
      # for a mulle project is the single amalgamated header directory set up
      # by AddSubdirectorySupport, and it establishes the build order, so the
      # headers are in place before we compile.
      #
      # In a mulle-craft build the dependencies are files, not targets, and
      # their headers come from DEPENDENCY_DIR, so nothing happens here.
      #
      # HEADER_ONLY_LIBRARIES is in the list because a header only dependency
      # is usually an INTERFACE target. Linking one adds nothing to the link
      # line, it only transfers the include directories and the dependency on
      # the targets that stage the headers. Without it we would race the
      # staging and fail to find its headers.
      #
      foreach( _dep_lib
               ${DEPENDENCY_LIBRARIES}
               ${OPTIONAL_DEPENDENCY_LIBRARIES}
               ${ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}
               ${FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${STARTUP_DEPENDENCY_LIBRARIES}
               ${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${HEADER_ONLY_LIBRARIES})
         if( TARGET "${_dep_lib}")
            message( STATUS "${LIBRARY_COMPILE_TARGET} inherits usage requirements of target \"${_dep_lib}\"")
            target_link_libraries( ${LIBRARY_COMPILE_TARGET} PRIVATE "${_dep_lib}")
         endif()
      endforeach()
      unset( _dep_lib)
   else()
      set( LIBRARY_COMPILE_TARGET "${LIBRARY_NAME}")
      set( LIBRARY_LINK_TARGET "${LIBRARY_NAME}")
   endif()


   if( STAGE2_SOURCES)
      set( LIBRARY_STAGE2_TARGET "_2_${LIBRARY_NAME}")

      add_library( ${LIBRARY_STAGE2_TARGET} OBJECT
         ${STAGE2_SOURCES}
         ${STAGE2_HEADERS}
      )
      set( ALL_OBJECT_FILES
         ${ALL_OBJECT_FILES}
         $<TARGET_OBJECTS:${LIBRARY_STAGE2_TARGET}>
      )

      # set on all stages
      set_target_properties( ${LIBRARY_STAGE2_TARGET}
         PROPERTIES
            CXX_STANDARD 11
      )
      target_compile_definitions( ${LIBRARY_STAGE2_TARGET} PRIVATE "${LIBRARY_UPCASE_IDENTIFIER}_BUILD")
      if( BUILD_SHARED_LIBS)
         set_property( TARGET ${LIBRARY_STAGE2_TARGET} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
      endif()

      #
      # OVERRIDE FIX (cmake/Library.cmake, precedes cmake/share on module path):
      #
      # Mirror the stage-1 "add_subdirectory support" block for stage 2. The
      # upstream cmake/share/Library.cmake applies dependency-target linking
      # ONLY to the stage-1 OBJECT library (_1_${LIBRARY_NAME}); stage 2
      # (_2_${LIBRARY_NAME}) never links the dependency targets, so it does not
      # inherit their INTERFACE_INCLUDE_DIRECTORIES (the shared amalgamated
      # header dir) nor the build ordering. In a mulle-craft / prebuilt-SDK
      # build that is invisible because headers come from DEPENDENCY_DIR via
      # -isystem. But for an external consumer that builds the dependencies from
      # source with add_subdirectory() (no SDK), stage-2 sources such as
      # MulleObjC*Foundation that #include <mulle-objc-runtime/...> fail to find
      # the headers. Linking the resolved dependency targets here gives stage 2
      # the same usage requirements and ordering as stage 1.
      #
      foreach( _dep_lib
               ${DEPENDENCY_LIBRARIES}
               ${OPTIONAL_DEPENDENCY_LIBRARIES}
               ${ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}
               ${FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${STARTUP_DEPENDENCY_LIBRARIES}
               ${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
               ${HEADER_ONLY_LIBRARIES})
         if( TARGET "${_dep_lib}")
            message( STATUS "${LIBRARY_STAGE2_TARGET} inherits usage requirements of target \"${_dep_lib}\"")
            target_link_libraries( ${LIBRARY_STAGE2_TARGET} PRIVATE "${_dep_lib}")
         endif()
      endforeach()
      unset( _dep_lib)
   else()
      if( STAGE2_HEADERS)
         message( SEND_ERROR "No STAGE2_SOURCES found but STAGE2_HEADERS exist")
      endif()
      unset( LIBRARY_STAGE2_TARGET)
   endif()

   get_property( _mulle_header_staging_targets GLOBAL
                 PROPERTY MULLE_HEADER_STAGING_TARGETS)
   if( _mulle_header_staging_targets)
      foreach( _staging_target ${_mulle_header_staging_targets})
         if( TARGET "${_staging_target}")
            string( MAKE_C_IDENTIFIER "${_staging_target}"
                    _staging_identifier)
            set( _staging_stamp
                 "${CMAKE_CURRENT_BINARY_DIR}/.${_staging_identifier}-headers-staged")
            add_custom_command(
               OUTPUT "${_staging_stamp}"
               COMMAND ${CMAKE_COMMAND} -E touch "${_staging_stamp}"
               DEPENDS "${_staging_target}"
               COMMENT "Waiting for ${_staging_target} headers"
            )
            foreach( _source ${LIBRARY_SOURCES} ${STAGE2_SOURCES})
               set_property( SOURCE "${_source}" APPEND
                             PROPERTY OBJECT_DEPENDS "${_staging_stamp}")
            endforeach()
         endif()
      endforeach()
   endif()
   unset( _mulle_header_staging_targets)
   unset( _staging_target)
   unset( _staging_identifier)
   unset( _staging_stamp)

   if( LINK_PHASE)

      #
      # Three ways to do shared libraries:
      #
      # * define -DBUILD_SHARED_LIBS=ON on the cmake command line and keep the file
      #   as is.
      # * Let mulle-make do this for you with:
      #      mulle-sde definition --global set BUILD_SHARED_LIBS ON
      # * Define  LIBRARY_NAME_TYPE as SHARED
      #      set( BUILD_SHARED_LIBS ON) line.
      #
      add_library( "${LIBRARY_NAME}"
         ${ALL_OBJECT_FILES}
         ${INSTALL_PUBLIC_HEADERS}  # else won't get installed by framework
         ${INSTALL_PRIVATE_HEADERS} # else won't get installed by framework
         ${LIBRARY_RESOURCES}
      )

      add_dependencies( "${LIBRARY_NAME}" ${LIBRARY_COMPILE_TARGET})
      if( STAGE2_SOURCES)
         add_dependencies( "${LIBRARY_NAME}" ${LIBRARY_STAGE2_TARGET})
      endif()

      #
      # allow forward definitions in shared library
      #
      option( SHARED_UNRESOLVED_SYMBOLS "Shared libraries may have unresolved symbols" ON)
      # output library with 'd' suffix when in windows (and creating a debug lib)
      if( MSVC)
         set_target_properties( "${LIBRARY_NAME}" PROPERTIES DEBUG_POSTFIX "d")
      endif()

      include( LibraryAux OPTIONAL)

      if( BUILD_SHARED_LIBS)
         if( SHARED_UNRESOLVED_SYMBOLS)
            if( APPLE)
               target_link_libraries( "${LIBRARY_NAME}"
                  PRIVATE "-undefined dynamic_lookup"
               )
            endif()
         endif()

         if( NOT SHARED_LIBRARY_LIST)
            set( SHARED_LIBRARY_LIST
               ${DEPENDENCY_LIBRARIES}
               ${DEPENDENCY_FRAMEWORKS}
               ${OPTIONAL_DEPENDENCY_LIBRARIES}
               ${OPTIONAL_DEPENDENCY_FRAMEWORKS}
               ${OS_SPECIFIC_LIBRARIES}
               ${OS_SPECIFIC_FRAMEWORKS}
         )
         endif()

         include( PostSharedLibrary OPTIONAL) # additional hook

         target_link_libraries( "${LIBRARY_NAME}"
            PRIVATE ${SHARED_LIBRARY_LIST}
         )

         #
         # Something to set for shared libraries
         #
         # set_target_properties( "${LIBRARY_NAME}" PROPERTIES VERSION $ENV{PROJECT_VERSION})
         # set_target_properties( "${LIBRARY_NAME}" PROPERTIES SOVERSION 1)
         #
      endif()

      set( INSTALL_LIBRARY_TARGETS
         "${LIBRARY_NAME}"
         ${INSTALL_LIBRARY_TARGETS}
      )

      set( INSTALL_${LIBRARY_UPCASE_IDENTIFIER}_RESOURCES ${LIBRARY_RESOURCES})

   endif()
else()
   # header only library
   add_library("${LIBRARY_NAME}" INTERFACE
   )
endif()

include( PostLibrary OPTIONAL)

# clean LIBRARY_SOURCES for the next run, if set by this script
if( __LIBRARY_SOURCES_UNSET )
   unset( LIBRARY_SOURCES)
   unset( __LIBRARY_SOURCES_UNSET)
endif()


# clean LIBRARY_RESOURCES for the next run, if set by this script
if( __LIBRARY_RESOURCES_UNSET )
   unset( LIBRARY_RESOURCES)
   unset( __LIBRARY_RESOURCES_UNSET)
endif()
