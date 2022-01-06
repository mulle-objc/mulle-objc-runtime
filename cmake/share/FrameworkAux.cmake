### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


option( REEXPORT_ALL_LOAD "Reexport linked static Objective-C libraries (all-load) of framework" ON)

if( REEXPORT_ALL_LOAD)
  set( ALL_LOAD_PREFIX "-Xlinker -reexport_library ")  # space important
endif()


include( AllLoadC)
include( StandaloneC)

include( FrameworkAuxC OPTIONAL)


# extension : mulle-c/c-cmake
# directory : project/all
# template  : .../FrameworkAux.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
