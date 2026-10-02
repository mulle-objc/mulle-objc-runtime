### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# Executable.cmake
#    Creates the executable target. Like Library it compiles into an OBJECT
#    target first, then links in LINK_PHASE only.
#
# INPUT
#    PROJECT_NAME             fallback for EXECUTABLE_NAME
#    EXECUTABLE_NAME          name of the executable target
#    EXECUTABLE_SOURCES       defaults to SOURCES. An empty list is an error
#    OTHER_EXECUTABLE_OBJECT_FILES                       pre-compiled objects
#    OTHER_<EXECUTABLE_UPCASE_IDENTIFIER>_OBJECT_FILES   likewise
#    EXECUTABLE_DEPENDENCY_NAMES  extra add_dependencies targets
#    PROJECT_HEADERS          added to the target for IDE visibility
#    CMAKE_EDITABLE_FILES     likewise
#    LINK_PHASE               gates add_executable and all linking
#    DEPENDENCY_LIBRARIES     and the other lists from Dependencies
#    HEADER_ONLY_LIBRARIES    from Dependencies. Entries that are cmake
#                             targets are linked for their include
#                             directories and build order only
#    MULLE_TRACE_INCLUDE      trace inclusion of this file
#
# OPTIONS
#    EXE_EXPORT_ALL     ON  sets CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
#    EXE_UTF8_MANIFEST  ON  on Windows embed an application manifest that
#                           declares the active code page as UTF-8, so that
#                           argv and the environment arrive as UTF-8
#
# OUTPUT
#    EXECUTABLE_IDENTIFIER           snake cased EXECUTABLE_NAME
#    EXECUTABLE_UPCASE_IDENTIFIER    upper cased variant
#    EXECUTABLE_DOWNCASE_IDENTIFIER  lower cased variant
#    EXECUTABLE_COMPILE_TARGET       target carrying compile options
#    EXECUTABLE_LINK_TARGET          target carrying link options
#    INSTALL_EXECUTABLE_TARGETS      consumed by InstallExecutable
#    CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS  from EXE_EXPORT_ALL
#
#    Defines <EXECUTABLE_UPCASE_IDENTIFIER>_BUILD while compiling, sets
#    ENABLE_EXPORTS, and on APPLE an INSTALL_RPATH/BUILD_RPATH pointing at
#    ../lib and ../Frameworks.
#
# INCLUDES
#    StringCase, PreExecutable (OPTIONAL), Rpath (OPTIONAL),
#    ExecutableAux (OPTIONAL)
#
# GUARD
#    none. Include once per executable
#

# This can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( NOT EXECUTABLE_NAME)
   set( EXECUTABLE_NAME "${PROJECT_NAME}")
endif()

include( StringCase)

if( NOT EXECUTABLE_IDENTIFIER)
   snakeCaseString( "${EXECUTABLE_NAME}" EXECUTABLE_IDENTIFIER)
endif()
if( NOT EXECUTABLE_UPCASE_IDENTIFIER)
   string( TOUPPER "${EXECUTABLE_IDENTIFIER}" EXECUTABLE_UPCASE_IDENTIFIER)
endif()
if( NOT EXECUTABLE_DOWNCASE_IDENTIFIER)
   string( TOLOWER "${EXECUTABLE_IDENTIFIER}" EXECUTABLE_DOWNCASE_IDENTIFIER)
endif()


if( NOT EXECUTABLE_SOURCES)
   set( EXECUTABLE_SOURCES "${SOURCES}")
   set( __EXECUTABLE_SOURCES_UNSET ON)
endif()

option( EXE_EXPORT_ALL "Export all global symbols for executable" ON)
set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ${EXE_EXPORT_ALL})

option( EXE_UTF8_MANIFEST "Embed a UTF-8 activeCodePage manifest into the executable" ON)

#
# must be ahead of AllLoadC
#
include( PreExecutable OPTIONAL)

if( NOT EXECUTABLE_SOURCES)
   message( SEND_ERROR "There are no sources to compile for executable ${EXECUTABLE_NAME}. Did `mulle-sde reflect` run yet ?")
endif()


set( EXECUTABLE_COMPILE_TARGET "_1_${EXECUTABLE_NAME}")
set( EXECUTABLE_LINK_TARGET "${EXECUTABLE_NAME}")

#
# In the compile-only phase the executable target does not exist yet (it is
# created below under LINK_PHASE). Point the link target at the compile
# target, so unguarded target_link_options() calls in project CMakeLists.txt
# do not fail. An OBJECT library does not link, so the options are ignored.
#
if( NOT LINK_PHASE)
   set( EXECUTABLE_LINK_TARGET "${EXECUTABLE_COMPILE_TARGET}")
endif()

add_library( ${EXECUTABLE_COMPILE_TARGET} OBJECT
   ${EXECUTABLE_SOURCES}
)

set( ALL_OBJECT_FILES
   $<TARGET_OBJECTS:${EXECUTABLE_COMPILE_TARGET}>
   ${OTHER_EXECUTABLE_OBJECT_FILES}
   ${OTHER_${EXECUTABLE_UPCASE_IDENTIFIER}_OBJECT_FILES}
)

set_target_properties( ${EXECUTABLE_COMPILE_TARGET}
   PROPERTIES
      CXX_STANDARD 11
#      DEFINE_SYMBOL "${EXECUTABLE_UPCASE_IDENTIFIER}_SHARED_BUILD"
)

target_compile_definitions( ${EXECUTABLE_COMPILE_TARGET} PRIVATE "${EXECUTABLE_UPCASE_IDENTIFIER}_BUILD")

#
# add_subdirectory support: same as in Library.cmake.
#
# An OBJECT library does not link, so it does not pick up the usage
# requirements (include directories, definitions) of our dependencies by
# itself. When a dependency resolved to a cmake target - which is what
# happens when it was pulled in with add_subdirectory - link against it.
# That gives us its INTERFACE_INCLUDE_DIRECTORIES and establishes the build
# order, so the headers are in place before we compile.
#
# In a mulle-craft build the dependencies are files, not targets, and their
# headers come from DEPENDENCY_DIR, so nothing happens here.
#
# HEADER_ONLY_LIBRARIES is in the list because a header only dependency is
# usually an INTERFACE target. Linking one adds nothing to the link line, it
# only transfers the include directories and the dependency on the targets
# that stage the headers.
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
      message( STATUS "${EXECUTABLE_COMPILE_TARGET} inherits usage requirements of target \"${_dep_lib}\"")
      target_link_libraries( ${EXECUTABLE_COMPILE_TARGET} PRIVATE "${_dep_lib}")
   endif()
endforeach()
unset( _dep_lib)


if( LINK_PHASE)
   # RPATH must be ahead of add_executable
   include( Rpath OPTIONAL)

   #
   # Windows derives argv and the environment from the wide command line
   # using the ANSI code page (GetACP), so a program that reads UTF-8 gets
   # mojibake and the original bytes are already lost. An application
   # manifest declaring activeCodePage UTF-8 fixes this on Windows 10 1903
   # and newer. It is embedded as an executable resource (id 1 is the
   # manifest the loader reads for an executable, type 24 is RT_MANIFEST),
   # which makes it a per-executable concern, not something a linked library
   # can provide. Older Windows ignores the setting.
   #
   set( UTF8_MANIFEST_SOURCES)

   if( WIN32 AND EXE_UTF8_MANIFEST)
      enable_language( RC)

      set( _utf8_manifest "${CMAKE_CURRENT_BINARY_DIR}/utf8-executable.manifest")
      set( _utf8_rc       "${CMAKE_CURRENT_BINARY_DIR}/utf8-executable.rc")

      file( WRITE "${_utf8_manifest}"
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">
  <application>
    <windowsSettings>
      <activeCodePage xmlns=\"http://schemas.microsoft.com/SMI/2019/WindowsSettings\">UTF-8</activeCodePage>
    </windowsSettings>
  </application>
</assembly>
")

      # numbers spelled out for any windres, including llvm-windres
      file( WRITE "${_utf8_rc}"
"1 24 \"utf8-executable.manifest\"
")

      set_source_files_properties( "${_utf8_rc}" PROPERTIES
         COMPILE_OPTIONS "--include-dir=${CMAKE_CURRENT_BINARY_DIR}")

      set( UTF8_MANIFEST_SOURCES "${_utf8_rc}")
   endif()

   add_executable( "${EXECUTABLE_NAME}"
      ${ALL_OBJECT_FILES}
      ${PROJECT_HEADERS}
      ${CMAKE_EDITABLE_FILES}
      ${UTF8_MANIFEST_SOURCES}
   )

   add_dependencies( "${EXECUTABLE_NAME}"
      ${EXECUTABLE_COMPILE_TARGET}
      ${EXECUTABLE_DEPENDENCY_NAMES}
   )

   # useful for mulle-c, but can be commented out
   set_target_properties( "${EXECUTABLE_NAME}"
      PROPERTIES
         CXX_STANDARD 11
#         DEFINE_SYMBOL "${EXECUTABLE_UPCASE_IDENTIFIER}_SHARED_BUILD"
   )

   #
   # not __really__ sure why this seems to be need now (cmake >= 4)
   # used to work fine without
   #
   if( APPLE)
      set_target_properties("${EXECUTABLE_NAME}" PROPERTIES
          INSTALL_RPATH "@loader_path/../lib/;@loader_path/../Frameworks/"
          BUILD_RPATH "@loader_path/../lib/;@loader_path/../Frameworks/"
      )
   endif()

   set_target_properties("${EXECUTABLE_NAME}" PROPERTIES ENABLE_EXPORTS ON)

   target_compile_definitions( "${EXECUTABLE_NAME}" PRIVATE "${EXECUTABLE_UPCASE_IDENTIFIER}_BUILD")

   #
   # this will set EXECUTABLE_LIBRARY_LIST if ALL_LOAD is used
   # and EXECUTABLE_LIBRARY_LIST is not set yet
   #
   include( ExecutableAux OPTIONAL)

   #
   # fall back if EXECUTABLE_LIBRARY_LIST is not set by ALL_LOAD
   #
   if( NOT DEFINED EXECUTABLE_LIBRARY_LIST)
      if( ALL_LOAD_DEPENDENCY_LIBRARIES)
         message( FATAL_ERROR "ALL_LOAD_DEPENDENCY_LIBRARIES \
\"${ALL_LOAD_DEPENDENCY_LIBRARIES}\" are not linked to ${EXECUTABLE_NAME}.
If these are regular C libraries, be sure, that they are marked as
\"no-all-load\" in the sourcetree and inherited sourcetrees.

  mulle-sde dependency unmark <name> all-load
")
      endif()

      if( FORCE_ALL_LOAD_DEPENDENCY_FRAMEWORKS)
         message( FATAL_ERROR "FORCE_ALL_LOAD_DEPENDENCY_FRAMEWORKS \
\"${FORCE_ALL_LOAD_DEPENDENCY_FRAMEWORKS}\" are not linked to ${EXECUTABLE_NAME}.
Frameworks aren't force loaded.")
      endif()


      if( ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES)
         message( FATAL_ERROR "ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES \
\"${ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}\" are not linked to ${EXECUTABLE_NAME}.
If these are regular C libraries, be sure, that they are marked as
\"no-all-load\" in the sourcetree and inherited sourcetrees.

  mulle-sde dependency unmark <name> all-load
")
      endif()

      if( FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS)
         message( FATAL_ERROR "FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS \
\"${FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS}\" are not linked to ${EXECUTABLE_NAME}.
Frameworks aren't force loaded.")
      endif()


      if( ALL_LOAD_OS_SPECIFIC_LIBRARIES)
         message( FATAL_ERROR "ALL_LOAD_OS_SPECIFIC_LIBRARIES \
\"${ALL_LOAD_OS_SPECIFIC_LIBRARIES}\" are not linked to ${EXECUTABLE_NAME}.
If these are regular C libraries, be sure, that they are marked as
\"no-all-load\" in the sourcetree and inherited sourcetrees.

  mulle-sde dependency unmark <name> all-load
")
      endif()

      if( FORCE_ALL_LOAD_OS_SPECIFIC_FRAMEWORKS)
         message( FATAL_ERROR "FORCE_ALL_LOAD_OS_SPECIFIC_FRAMEWORKS \
\"${FORCE_ALL_LOAD_OS_SPECIFIC_FRAMEWORKS}\" are not linked to ${EXECUTABLE_NAME}.
Frameworks aren't force loaded.")
      endif()

      # mulle-atinit and mulle-atexit end up here, not good if you
      # just used mulle-sde/c-developer
      if( STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES)
         message( FATAL_ERROR "STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES \
\"${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}\" are not linked to ${EXECUTABLE_NAME}. \
STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES is a mulle-c/c-developer feature, but this \
project is seemingly not setup for it.")
      endif()

      if( FORCE_STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS)
         message( FATAL_ERROR "FORCE_STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS \
\"${FORCE_STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS}\" are not linked to ${EXECUTABLE_NAME}.
Frameworks aren't force loaded.")

      endif()

      # MEMO: some of these definitions may not exist like
      #       STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS we just keep them
      #       for orthogonality
      #       Frameworks aren't forced and can't be forced
      set( EXECUTABLE_LIBRARY_LIST
         ${FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES}
         ${ALL_LOAD_DEPENDENCY_FRAMEWORKS}
         ${DEPENDENCY_LIBRARIES}
         ${DEPENDENCY_FRAMEWORKS}

         ${FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}
         ${ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
         ${OPTIONAL_DEPENDENCY_FRAMEWORKS}

         ${FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
         ${STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS}
         ${STARTUP_DEPENDENCY_LIBRARIES}
         ${STARTUP_DEPENDENCY_FRAMEWORKS}

         ${FORCE_ALL_LOAD_OS_SPECIFIC_LIBRARIES}
         ${ALL_LOAD_OS_SPECIFIC_FRAMEWORKS}
         ${OS_SPECIFIC_LIBRARIES}
         ${OS_SPECIFIC_FRAMEWORKS}
      )
   endif()

   # MEMO: its probably better to not have a big EXECUTABLE_LIBRARY_LIST but
   #       instead break it up into multiple target_link_libraries calls, 
   #       where we can set the WHOLEARCHIVE property separately
   #
   #       foreach( library ${ALL_LOAD_DEPENDENCY_LIBRARIES})
   #          target_link_libraries( "${EXECUTABLE_NAME}" "$<LINK_LIBRARY:WHOLE_ARCHIVE,${library}>")
   #       endforeach()   
   target_link_libraries( "${EXECUTABLE_NAME}"
      ${EXECUTABLE_LIBRARY_LIST}
   )

   set( INSTALL_EXECUTABLE_TARGETS
      "${EXECUTABLE_NAME}"
      ${INSTALL_EXECUTABLE_TARGETS}
   )

   if( EXECUTABLE_RESOURCES)
      set( INSTALL_${EXECUTABLE_UPCASE_IDENTIFIER}_RESOURCES ${EXECUTABLE_RESOURCES})
   else()
      if( RESOURCES)
         set( INSTALL_${EXECUTABLE_UPCASE_IDENTIFIER}_RESOURCES ${RESOURCES})
      endif()
   endif()

   include( PostExecutable OPTIONAL)

   # clean EXECUTABLE_SOURCES for the next run, if set by this script
   if( __EXECUTABLE_SOURCES_UNSET )
      unset( EXECUTABLE_SOURCES)
      unset( __EXECUTABLE_SOURCES_UNSET)
   endif()
endif()
