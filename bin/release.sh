#! /bin/sh

PROJECT="MulleObjcRuntime"    # requires camel-case
DESC="An Objective-C runtime, written 100% in C"
DEPENDENCIES='${DEPENDENCY_TAP}mulle-concurrent
${DEPENDENCY_TAP}mulle-vararg'  # no camel case, will be evaled later!
LANGUAGE=c                    # c,cpp, objc

HEADER="src/mulle_objc_version.h"
VERSIONNAME="MULLE_OBJC_RUNTIME_VERSION"
NAME="mulle-objc-runtime"

#
# Ideally you don't hafta change anything below this line
#
# source mulle-homebrew.sh (clumsily)

PUBLISHER="mulle-nat"
PUBLISHER_TAP="mulle-kybernetik/software/"
DEPENDENCY_TAP="mulle-kybernetik/software/"
BOOTSTRAP_TAP="mulle-kybernetik/alpha/"


MULLE_BOOTSTRAP_FAIL_PREFIX="release.sh"
DIR="`dirname -- "$0"`"
. ${DIR}/mulle-homebrew/mulle-homebrew.sh || exit 1
cd "${DIR}/.."

# parse options
homebrew_parse_options "$@"

# dial past options
while [ $# -ne 0 ]
do
   case "$1" in
      -*)
         shift # assume treated by homebrew options
      ;;

      --*)
         shift # assume treated by homebrew options
         shift # assume treated by homebrew options
      ;;

      *)
         break;
      ;;
   esac
done



#
# this can usually be deduced, if you follow the conventions
#
VERSION="`get_header_version "${HEADER}" "${VERSIONNAME}"`"

# --- GIT ---
# tag to tag your release
# and the origin where
TAG="${TAG:-${TAGPREFIX}${VERSION}}"


# --- HOMEBREW TAP ---
# Specify to where and under what bame to publish via your brew tap
#
RBFILE="${NAME}.rb"                  # ruby file for brew
HOMEBREWTAP="../homebrew-`basename -- ${PUBLISHER_TAP}`"     # your tap repository path


main()
{
   git_main "${ORIGIN}" "${TAG}" || exit 1
   homebrew_main
}

main "$@"
