### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __PRE_FILES__CMAKE__)
   set( __PRE_FILES__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( CMakeTweaksC)

   # a place to add stuff for ObjC or C++
   include( PreFilesAuxC OPTIONAL)

endif()
