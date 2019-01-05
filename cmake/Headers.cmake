#
# The following includes include definitions generated
# during `mulle-sde update`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Headers)

# === MULLE-SDE END ===
#

#
# If you don't like the "automatic" way of generating _Headers
#
# MULLE_MATCH_TO_CMAKE_HEADERS_FILE="DISABLE" # or NONE
#

#
# Add ignored headers back in so that the generators pick them up
#
set( PUBLIC_HEADERS
"src/_mulle-objc-runtime-include.h"
${PUBLIC_HEADERS}
)

# keep headers to install separate to make last minute changes
set( INSTALL_PUBLIC_HEADERS ${PUBLIC_HEADERS})

#
# Do not install generated private headers and include-private.h
# which aren't valid outside of the project scope.
#
set( INSTALL_PRIVATE_HEADERS ${PRIVATE_HEADERS})
list( REMOVE_ITEM INSTALL_PRIVATE_HEADERS "include-private.h")

# add ignored headers back in so that the generators pick them up
set( PRIVATE_HEADERS
"src/_mulle-objc-runtime-include-private.h"
${PRIVATE_HEADERS}
)

#
# You can put more source and resource file definitions here.
#
