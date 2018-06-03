if( NOT __PRE_EXECUTABLE__CMAKE__)
   set( __PRE_EXECUTABLE__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( PreExecutableCAux OPTIONAL)
endif()
