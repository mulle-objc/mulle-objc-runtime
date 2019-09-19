#
# The following includes include definitions generated
# during `mulle-sde update`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Dependencies)
include( _Libraries)

# === MULLE-SDE END ===
#

#
# If you need more find_library() statements, that you dont want to manage
# with the sourcetree, add them here.
#


#
# This will be overridden by MulleObjC, which in turn will get overridden
# by MulleFoundation, each announcing their startup library.
#
# This library will be find_library as STARTUP_LIBRARY and linked against
# executables
#
set( STARTUP_LIBRARY_NAME "mulle-objc-runtime-startup")
