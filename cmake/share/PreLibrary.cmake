# can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

include( UnwantedWarningsC)
include( CompilerDetectionC)
include( CompilerFlagsC)

include( PreLibraryAuxC OPTIONAL)
