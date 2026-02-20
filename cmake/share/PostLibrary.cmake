### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( BUILD_SHARED_LIBS)
   target_compile_definitions( ${LIBRARY_COMPILE_TARGET} PRIVATE "MULLE_INCLUDE_DYNAMIC")
   if( LIBRARY_STAGE2_TARGET)
      target_compile_definitions( ${LIBRARY_STAGE2_TARGET} PRIVATE "MULLE_INCLUDE_DYNAMIC")
   endif()
endif()

include( PostLibraryAuxC OPTIONAL)
