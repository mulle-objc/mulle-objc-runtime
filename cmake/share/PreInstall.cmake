if( NOT __PRE_INSTALL__CMAKE__)
   set( __PRE_INSTALL__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( FinalOutputC)

   # a place to add stuff for ObjC or C++

   include( PreInstallCAux OPTIONAL)
endif()
