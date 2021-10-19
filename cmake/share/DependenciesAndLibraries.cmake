### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# Get Libraries first. That way local library definitions override those
# we might inherit from dependencies. The link order should not be affected by
# this.
#
include( _Libraries OPTIONAL)
include( _Dependencies OPTIONAL)

#
# This is an experiment to have "flat" headers. I don't use it though (yet)
#
option( INHERIT_DEPENDENCY_INCLUDES "Make headers of dependencies available as local headers" OFF)

if( INHERIT_DEPENDENCY_INCLUDES)
   # message( STATUS "INHERITED_INCLUDE_DIRS=\"${INHERITED_INCLUDE_DIRS}\"" )

   # these generate -I arguments, that add to the user search path
   include_directories( ${INHERITED_INCLUDE_DIRS})
endif()
