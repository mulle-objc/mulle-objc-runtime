### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# The following includes include definitions generated
# during `mulle-sde reflect`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Resources OPTIONAL)

# === MULLE-SDE END ===
#
#
# If you don't like the "automatic" way of generating _Resources
#
# MULLE_MATCH_TO_CMAKE_RESOURCES_FILE="DISABLE" # or NONE
#

#
# You can put more source and resource file definitions here.
#
set( INSTALL_RESOURCES ${RESOURCES})


# extension : mulle-sde/c-cmake
# directory : project/all
# template  : .../Resources.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
