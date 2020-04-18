#
# Git mirror and Zip/TGZ cache to conserve bandwidth
#
MULLE_FETCH_MIRROR_DIR="${MULLE_FETCH_MIRROR_DIR:-${HOME:-/tmp}/Library/Caches/mulle-fetch/git-mirror}"
export MULLE_FETCH_MIRROR_DIR

MULLE_FETCH_ARCHIVE_DIR="${MULLE_FETCH_ARCHIVE_DIR:-${HOME:-/tmp}/Library/Caches/mulle-fetch/archive}"
export MULLE_FETCH_ARCHIVE_DIR
