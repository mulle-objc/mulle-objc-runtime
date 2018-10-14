if( NOT __POST_FILES__CMAKE__)
   set( __POST_FILES__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( PostFilesAuxC OPTIONAL)

endif()
