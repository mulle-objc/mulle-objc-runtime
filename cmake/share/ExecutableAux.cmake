### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


include( AllLoadC)

include( ExecutableAuxC OPTIONAL)

CreateForceAllLoadList( ALL_LOAD_DEPENDENCY_LIBRARIES FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES)
CreateForceAllLoadList( STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES)

if( NOT EXECUTABLE_LIBRARY_LIST)
   set( EXECUTABLE_LIBRARY_LIST
      ${FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES}
      ${DEPENDENCY_LIBRARIES}
      ${OPTIONAL_DEPENDENCY_LIBRARIES}
      ${FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
      ${STARTUP_DEPENDENCY_LIBRARIES}
      ${OS_SPECIFIC_LIBRARIES}
   )
endif()
