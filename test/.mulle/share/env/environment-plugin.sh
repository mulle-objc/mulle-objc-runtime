#
# Git mirror and Zip/TGZ cache to conserve bandwidth
# Memo: override in os-specific env file
# Can be overridden with -DMULLE_FETCH_ARCHIVE_DIR on the commandline
#
MULLE_FETCH_MIRROR_DIR="${MULLE_FETCH_MIRROR_DIR:-${HOME:-/tmp}/.cache/mulle-fetch/git-mirror}"
export MULLE_FETCH_MIRROR_DIR

#
# Git mirror and Zip/TGZ cache to conserve bandwidth
# Can be overridden with -D on the commandline
MULLE_FETCH_ARCHIVE_DIR="${MULLE_FETCH_ARCHIVE_DIR:-${HOME:-/tmp}/.cache/mulle-fetch/archive}"
export MULLE_FETCH_ARCHIVE_DIR

#
# PATH to search for git repositories locally.
# Can be overridden with -DMULLE_FETCH_SEARCH_PATH on the commandline
#
MULLE_FETCH_SEARCH_PATH="${MULLE_FETCH_SEARCH_PATH:-${MULLE_VIRTUAL_ROOT}/..}"
export MULLE_FETCH_SEARCH_PATH

#
# Prefer symlinks to clones of git repos found in MULLE_FETCH_SEARCH_PATH
# Can be overridden with -DMULLE_SOURCETREE_SYMLINK on the commandline
#
MULLE_SOURCETREE_SYMLINK="${MULLE_SOURCETREE_SYMLINK:-YES}"
export MULLE_SOURCETREE_SYMLINK

#
# Use common folder for sharable projects.
# Can be overridden with -MULLE_SOURCETREE_STASH_DIRNAME on the commandline
#
MULLE_SOURCETREE_STASH_DIRNAME="${MULLE_SOURCETREE_STASH_DIRNAME:-stash}"
export MULLE_SOURCETREE_STASH_DIRNAME

#
# Share dependency directory (absolute for ease of use)
# Can be overridden with -DDEPENDENCY_DIR on the commandline
#
DEPENDENCY_DIR="${DEPENDENCY_DIR:-${MULLE_VIRTUAL_ROOT}/dependency}"
export DEPENDENCY_DIR

#
# Share addiction directory (absolute for ease of use)
# Can be overridden with -DADDICTION_DIR on the commandline
#
ADDICTION_DIR="${ADDICTION_DIR:-${MULLE_VIRTUAL_ROOT}/addiction}"
export ADDICTION_DIR

#
# Use common build directory
# Can be overridden with -DKITCHEN_DIR on the commandline
#
KITCHEN_DIR="${KITCHEN_DIR:-${MULLE_VIRTUAL_ROOT}/kitchen}"
export KITCHEN_DIR
#
#
#
export MULLE_SDE_INSTALLED_VERSION="0.44.1"


