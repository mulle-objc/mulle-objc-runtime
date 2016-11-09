#! /bin/sh

PROJECT="MulleObjcRuntime"    # requires camel-case
DESC="An Objective-C runtime, written 100% in C"
DEPENDENCIES='${DEPENDENCY_TAP}/mulle-concurrent
${DEPENDENCY_TAP}/mulle-vararg'  # no camel case, will be evaled later!
LANGUAGE=c                    # c,cpp, objc

#
# Ideally you don't hafta change anything below this line
#
# source mulle-homebrew.sh (clumsily)
MULLE_BOOTSTRAP_FAIL_PREFIX="release.sh"

. ./bin/repository-info.sh || exit 1
. ./bin/mulle-homebrew/mulle-homebrew.sh || exit 1

# parse options
homebrew_parse_options "$@"

# dial past options
while [ $# -ne 0 ]
do
   case "$1" in
      -*)
         shift
      ;;
      *)
         break;
      ;;
   esac
done


#
# these can usually be deduced, if you follow the conventions
#
HEADER="src/mulle_objc_version.h"
VERSIONNAME="MULLE_OBJC_RUNTIME_VERSION"
VERSION="`get_header_version "${HEADER}" "${VERSIONNAME}"`"

NAME="mulle-objc"
HOMEPAGE="`eval echo "${HOMEPAGE}"`"
NAME="mulle-objc-runtime"


# --- HOMEBREW TAP ---
# Specify to where and under what bame to publish via your brew tap
#
RBFILE="${NAME}.rb"                    # ruby file for brew
HOMEBREWTAP="../homebrew-software"     # your tap repository path


# --- GIT ---
# tag to tag your release
# and the origin where
TAG="${1:-${TAGPREFIX}${VERSION}}"


main()
{
   git_main "${ORIGIN}" "${TAG}" || exit 1
   homebrew_main
}

main "$@"
