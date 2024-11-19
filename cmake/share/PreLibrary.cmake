### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

include( CompilerDetectionC)
include( CompilerWarningsC)   # after detection, before flags
include( CompilerFlagsC)

include( PreLibraryAuxC OPTIONAL)
