#
# These are used by mulle-match find to speed up the search.
#
export MULLE_MATCH_FIND_NAMES="*.stdout,*.stdin,*.stderr,*.errors,*.ccdiag"


#
# Make project to test discoverable via MULLE_FETCH_SEARCH_PATH
# We assume we are in ${PROJECT_DIR}/test so modify search path to add ../..
#
export MULLE_FETCH_SEARCH_PATH="${MULLE_FETCH_SEARCH_PATH}:${MULLE_VIRTUAL_ROOT}/../.."


