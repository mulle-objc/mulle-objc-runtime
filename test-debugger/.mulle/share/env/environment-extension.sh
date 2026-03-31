#
# Reset to empty
#
export MULLE_SDE_REFLECT_CALLBACKS=""


#
# Used by `mulle-match find` to speed up the search.
#
export MULLE_MATCH_FILENAMES="config*"


#
# Used by `mulle-match find` to locate files
#
export MULLE_MATCH_PATH=".mulle/etc/sourcetree"


#
# Used by `mulle-match find` to ignore boring subdirectories like .git
#
export MULLE_MATCH_IGNORE_PATH=""


#
# These are used by mulle-match find to speed up the search.
#
export MULLE_MATCH_FIND_NAMES="*.environment,*.args,*.stdout,*.stdin,*.stderr,*.errors,*.ccdiag"


#
# Make project to test discoverable via MULLE_FETCH_SEARCH_PATH
# We assume we are in ${PROJECT_DIR}/test so modify search path to add ../..
#
export MULLE_FETCH_SEARCH_PATH="${MULLE_FETCH_SEARCH_PATH}:${MULLE_VIRTUAL_ROOT}/../.."


#
# tests don't need a graveyard
#
export MULLE_SOURCETREE_GRAVEYARD_ENABLED="NO"


#
# For new projects its better to keep this on "ignore" first, and then change
# it to "warn" or "error" later
#
export MULLE_TEST_WHITESPACE_DIFFERENCES="ignore"


