#
# Used to be hardcoded in mulle-sde plugin, now part of the environment.
# pick first as default (if any)
#
export _MULLE_SDK_DIR="${DEPENDENCY_DIR:-${MULLE_VIRTUAL_ROOT}/dependency}/${MULLE_CRAFT_SDKS%%:*}"


#
#
#
export MULLE_SDK_DIR="${_MULLE_SDK_DIR%%/}"


#
#
#
export PATH="${MULLE_SDK_DIR}/Debug/bin:${MULLE_SDK_DIR}/Release/bin:${MULLE_SDK_DIR}/bin:${PATH}"


