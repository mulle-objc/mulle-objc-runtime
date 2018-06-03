#
# Git mirror and Zip/TGZ cache to conserve bandwidth
# Memo: override in os-specific env file
#
export MULLE_FETCH_MIRROR_DIR="${HOME:-/tmp}/.cache/mulle-fetch/git-mirror"

#
# Git mirror and Zip/TGZ cache to conserve bandwidth
#
export MULLE_FETCH_ARCHIVE_DIR="${HOME:-/tmp}/.cache/mulle-fetch/archive"

#
# PATH to search for git repositories locally
#
export MULLE_FETCH_SEARCH_PATH="${MULLE_VIRTUAL_ROOT}/.."

#
# Prefer symlinking to local git repositories found via MULLE_FETCH_SEARCH_PATH
#
export MULLE_SYMLINK="YES"

#
# Use common folder for sharable projects
#
export MULLE_SOURCETREE_SHARE_DIR="${MULLE_VIRTUAL_ROOT}/stash"

#
# Share dependency directory (absolute for ease of use)
#
export DEPENDENCY_DIR="${MULLE_VIRTUAL_ROOT}/dependency"

#
# Share addiction directory (absolute for ease of use)
#
export ADDICTION_DIR="${MULLE_VIRTUAL_ROOT}/addiction"

#
# Use common build directory
#
export BUILD_DIR="${MULLE_VIRTUAL_ROOT}/build"
# Used by `mulle-match find` to speed up the search.
export MULLE_MATCH_FIND_NAMES="config:*.h:*.inc:*.c:CMakeLists.txt:*.cmake"


# Used by `mulle-match find` to locate files
export MULLE_MATCH_FIND_LOCATIONS="${PROJECT_SOURCE_DIR}:CMakeLists.txt:cmake"


# By default assume a project has source and a sourcetree
# The order should be sourcetree then source


#
#
#
export MULLE_SDE_INSTALLED_VERSION="0.20.1"


# By default assume a project has source and a sourcetree
# The order should be sourcetree then source


# By default assume a project has source and a sourcetree
# The order should be sourcetree then source


# By default assume a project has source and a sourcetree
# The order should be sourcetree then source


# By default assume a project has source and a sourcetree
# The order should be sourcetree then source
export MULLE_SDE_UPDATE_CALLBACKS="sourcetree:source"


