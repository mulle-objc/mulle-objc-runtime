### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( NOT EXECUTABLE_NAME)
   set( EXECUTABLE_NAME "${PROJECT_NAME}")
endif()
if( NOT EXECUTABLE_IDENTIFIER)
   string( MAKE_C_IDENTIFIER "${EXECUTABLE_NAME}" EXECUTABLE_IDENTIFIER)
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

#
# must be ahead of AllLoadC
#
include( PreExecutable OPTIONAL)

if( NOT EXECUTABLE_SOURCES)
   message( FATAL_ERROR "There are no sources to compile for executable ${EXECUTABLE_NAME}. Did `mulle-sde reflect` run yet ?")
endif()


add_library( "_1_${EXECUTABLE_NAME}" OBJECT
   ${EXECUTABLE_SOURCES}
)

set( ALL_OBJECT_FILES
   $<TARGET_OBJECTS:_1_${EXECUTABLE_NAME}>
   ${OTHER_EXECUTABLE_OBJECT_FILES}
   ${OTHER_${EXECUTABLE_UPCASE_IDENTIFIER}_OBJECT_FILES}
)

set_property( TARGET "_1_${EXECUTABLE_NAME}" PROPERTY CXX_STANDARD 11)

# RPATH must be ahead of add_executable
include( InstallRpath OPTIONAL)

if( LINK_PHASE)
   add_executable( "${EXECUTABLE_NAME}"
      ${ALL_OBJECT_FILES}
      ${PROJECT_HEADERS}
      ${CMAKE_EDITABLE_FILES}
   )

   add_dependencies( "${EXECUTABLE_NAME}"
      "_1_${EXECUTABLE_NAME}"
      ${EXECUTABLE_DEPENDENCY_NAMES}
   )

   # useful for mulle-c, but can be commented out
   set_property( TARGET "${EXECUTABLE_NAME}" PROPERTY CXX_STANDARD 11)

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


      if( STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES)
         message( FATAL_ERROR "STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES \
\"${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}\" are not linked to ${EXECUTABLE_NAME}.
STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES is an Objective-C feature, but this
project is seemingly not setup for Objective-C.")
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
