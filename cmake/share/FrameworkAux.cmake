# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

include( AllLoadC)
include( StartupC)
include( StandaloneC)

include( FrameworkAuxC OPTIONAL)
