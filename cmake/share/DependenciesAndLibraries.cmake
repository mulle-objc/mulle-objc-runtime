### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# The following includes include definitions generated
# during `mulle-sde update`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Dependencies OPTIONAL)
include( _Libraries OPTIONAL)

# === MULLE-SDE END ===
#

#
# If you need more find_library() statements, that you dont want to manage
# with the sourcetree, add them here.
#
# Add OS specific dependencies to OS_SPECIFIC_LIBRARIES
# Add all other dependencies (rest) to DEPENDENCY_LIBRARIES
#
