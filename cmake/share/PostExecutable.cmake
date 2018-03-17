if( NOT __POST_EXECUTABLE__CMAKE__)
   set( __POST_EXECUTABLE__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( FinalOutput)
   include( PostExecutableCAux OPTIONAL)

endif()
