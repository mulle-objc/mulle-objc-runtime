if( NOT __PRE_LIBRARY__CMAKE__)
   set( __PRE_LIBRARY__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( PreLibraryCAux OPTIONAL)

endif()
