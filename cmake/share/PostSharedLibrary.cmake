if( NOT __POST_SHARED_LIBRARY_C_CMAKE__)
   set( __POST_SHARED_LIBRARY_C_CMAKE__ ON)

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

   include( SharedLibraryCAux OPTIONAL)

endif()
