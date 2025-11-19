#
# Git mirror and Zip/TGZ cache to conserve bandwidth
# Memo: Will often be overridden in an os-specific environment file
# Can be overridden with -DMULLE_FETCH_ARCHIVE_DIR on the commandline
#
export MULLE_FETCH_MIRROR_DIR="${HOME:-/tmp}/.cache/mulle-fetch/git-mirror"

#
# Git mirror and Zip/TGZ cache to conserve bandwidth
#
export MULLE_FETCH_ARCHIVE_DIR="${HOME:-/tmp}/.cache/mulle-fetch/archive"

#
# PATH to search for git repositories locally.
#
export MULLE_FETCH_SEARCH_PATH="${MULLE_VIRTUAL_ROOT}/.."

#
# Prefer symlinks to clones of git repos found in MULLE_FETCH_SEARCH_PATH
#
export MULLE_SOURCETREE_SYMLINK='YES'
#
#
#
export MULLE_SDE_INSTALLED_VERSION="3.5.0"


