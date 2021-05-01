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

include( _Headers OPTIONAL)

# === MULLE-SDE END ===
#

#
# If you don't like the "automatic" way of generating _Headers
#
# MULLE_MATCH_TO_CMAKE_HEADERS_FILE="DISABLE" # or NONE
#


# keep headers to install separate to make last minute changes
set( INSTALL_PUBLIC_HEADERS ${PUBLIC_HEADERS}
${PUBLIC_GENERIC_HEADERS}
${PUBLIC_GENERATED_HEADERS}
)

#
# Do not install generated private headers and include-private.h
# which aren't valid outside of the project scope.
#
set( INSTALL_PRIVATE_HEADERS ${PRIVATE_HEADERS})
if( INSTALL_PRIVATE_HEADERS)
   list( REMOVE_ITEM INSTALL_PRIVATE_HEADERS "include-private.h")
endif()

#
# You can put more source and resource file definitions here.
#
