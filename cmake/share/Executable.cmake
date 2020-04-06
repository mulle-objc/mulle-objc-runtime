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

if( NOT EXECUTABLE_SOURCES)
   set( EXECUTABLE_SOURCES "${SOURCES}")
   set( __EXECUTABLE_SOURCES_UNSET ON)
endif()

#
# must be ahead of AllLoadC
#
include( PreExecutable OPTIONAL)

if( NOT EXECUTABLE_SOURCES)
   message( FATAL_ERROR "There are no sources to compile for executable ${EXECUTABLE_NAME}. Did mulle-sde update run yet ?")
endif()


add_library( "_1_${EXECUTABLE_NAME}" OBJECT
   ${EXECUTABLE_SOURCES}
)

set( ALL_OBJECT_FILES
   $<TARGET_OBJECTS:_1_${EXECUTABLE_NAME}>
)

set_property( TARGET "_1_${EXECUTABLE_NAME}" PROPERTY CXX_STANDARD 11)


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

      if( STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES)
         message( FATAL_ERROR "STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES \
\"${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}\" are not linked to ${EXECUTABLE_NAME}.
STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES is an Objective-C feature, but this
project is seemingly not setup for Objective-C.")

      endif()

      set( EXECUTABLE_LIBRARY_LIST
         ${DEPENDENCY_LIBRARIES}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
         ${STARTUP_DEPENDENCY_LIBRARIES}
         ${OS_SPECIFIC_LIBRARIES}
      )
   endif()

   target_link_libraries( "${EXECUTABLE_NAME}"
      ${EXECUTABLE_LIBRARY_LIST}
   )

   set( INSTALL_EXECUTABLE_TARGETS
      "${EXECUTABLE_NAME}"
      ${INSTALL_EXECUTABLE_TARGETS}
   )

   include( PostExecutable OPTIONAL)

   # clean EXECUTABLE_SOURCES for the next run, if set by this script
   if( __EXECUTABLE_SOURCES_UNSET )
      unset( EXECUTABLE_SOURCES)
      unset( __EXECUTABLE_SOURCES_UNSET)
   endif()
endif()
