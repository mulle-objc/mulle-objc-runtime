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

#
# If we are in an IDE like CLion and the dependencies haven't been made yet
# cmake is unhappy, try to avoid that.
#
# When we are added with add_subdirectory there is no DEPENDENCY_DIR and there
# doesn't need to be one. Our dependencies are then expected to resolve to
# cmake targets, which _Dependencies looks for before it falls back to
# find_library. So run it in that case too.
#
if( IS_DIRECTORY "${DEPENDENCY_DIR}" OR NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
   include( _Dependencies OPTIONAL)
else()
   message( STATUS "DEPENDENCY_DIR \"${DEPENDENCY_DIR}\" is missing, so no dependencies")
endif()
