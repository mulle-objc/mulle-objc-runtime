#
# The following includes include definitions generated
# during `mulle-sde update`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Headers)
include( _Sources)

# === MULLE-SDE END ===
#

# add ignored header back in
set( PUBLIC_HEADERS
"src/_mulle-objc-runtime-dependencies.h"
${PUBLIC_HEADERS}
)

#
# You can put more source and resource file definitions here.
#
