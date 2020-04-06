### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

# included by PostLibrary already
# include( AllLoadC)

CreateForceAllLoadList( ALL_LOAD_DEPENDENCY_LIBRARIES FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES)

set( SHARED_LIBRARY_LIST
   ${FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES}
   ${SHARED_LIBRARY_LIST}
)

include( PostSharedLibraryAuxC OPTIONAL)
