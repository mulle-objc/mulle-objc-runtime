#! /usr/bin/env bash
#
#   Copyright (c) 2017 Nat! - Mulle kybernetiK
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#
#   Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
#   Neither the name of Mulle kybernetiK nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
#


# Define your project and the dependencies for homebrew
# DEPENDENCIES and BUILD_DEPENDENCIES will be evaled later!
# Then run this as
#   ./bin/release.sh --publisher mulle-nat --publisher-tap mulle-kybernetik/alpha/
#

# Define your project and the dependencies for homebrew
# DEPENDENCIES and BUILD_DEPENDENCIES will be evaled later!
# Then run this as
#   ./bin/release.sh --publisher mulle-nat --publisher-tap mulle-kybernetik/alpha/
#

PROJECT="MulleObjcRuntime"    # requires camel-case
DESC="An Objective-C runtime, written 100% in C"
LANGUAGE=c                    # c,cpp, objc

NAME="mulle-objc-runtime"


#
# Keep these commented out, if the automatic detection works well
# enough for you
#
VERSIONFILE="src/mulle_objc_version.h"
VERSIONNAME="MULLE_OBJC_RUNTIME_VERSION"

#
# Specify needed homebrew packages by name as you would when saying
# `brew install`.
#
# Use the ${DEPENDENCY_TAP} prefix for non-official dependencies.
#
DEPENDENCIES='${DEPENDENCY_TAP}mulle-concurrent
${DEPENDENCY_TAP}mulle-vararg'

BUILD_DEPENDENCIES='${BOOTSTRAP_TAP}mulle-bootstrap
${BOOTSTRAP_TAP}mulle-build
cmake'  # cmake would be useful to, if you are cmake based!


#######
# If you are using mulle-build, you don't hafta change anything after this
#######

#
# Generate your `def install` `test do` lines here to stdout.
#
generate_brew_formula_build()
{
   local project="$1"
   local name="$2"
   local version="$3"

   generate_brew_formula_mulle_build "${project}" "${name}" "${version}"
   generate_brew_formula_mulle_test  "${project}" "${name}" "${version}"
}


#
# If you are unhappy with the formula in general, then change
# this function. Print your formula to stdout.
#
generate_brew_formula()
{
#   local project="$1"
#   local name="$2"
#   local version="$3"
#   local dependencies="$4"
#   local builddependencies="$5"
#   local homepage="$6"
#   local desc="$7"
#   local archiveurl="$8"

   _generate_brew_formula "$@"
}

#######
# Ideally changes to the following values are done with the command line
# which makes it easier for forks.
#######

MULLE_BOOTSTRAP_FAIL_PREFIX="`basename -- $0`"
MULLE_HOMEBREW_VERSION="3.4.5"

EXEC_DIR="`dirname -- $0`"

#
# prefer local mulle-homebrew if available
#
if [ -x "${EXEC_DIR}/mulle-homebrew/mulle-homebrew-env" ]
then
   PATH="${EXEC_DIR}/mulle-homebrew:$PATH"
fi

if [ -z "`command -v mulle-homebrew-env`" ]
then
   echo "mulle-homebrew-env not found in PATH" >&2
   exit 1
fi


INSTALLED_MULLE_HOMEBREW_VERSION="`mulle-homebrew-env version`" || exit 1
LIBEXEC_DIR="`mulle-homebrew-env libexec-path`" || exit 1

. "${LIBEXEC_DIR}/mulle-homebrew.sh" || exit 1
. "${LIBEXEC_DIR}/mulle-git.sh"      || exit 1

if ! homebrew_is_compatible_version "${INSTALLED_MULLE_HOMEBREW_VERSION}" "${MULLE_HOMEBREW_VERSION}"
then
   fail "Installed mulle-homebrew version ${INSTALLED_MULLE_HOMEBREW_VERSION} is \
not compatible with this script from version ${MULLE_HOMEBREW_VERSION}"
fi

# parse options
homebrew_parse_options "$@"

#
# dial past options now as they have been parsed
#
while [ $# -ne 0 ]
do
   case "$1" in
      -*)
         shift
      ;;

      --*)
         shift
         shift
      ;;

      *)
         break;
      ;;
   esac
done

# --- FORMULA GENERATION ---

BOOTSTRAP_TAP="${BOOTSTRAP_TAP:-mulle-kybernetik/software/}"

DEPENDENCY_TAP="${DEPENDENCY_TAP:-${PUBLISHER_TAP}}"

#
# these can usually be deduced, if you follow the conventions
#
if [ -z "${NAME}" ]
then
   NAME="`get_name_from_project "${PROJECT}" "${LANGUAGE}"`"
fi

if [ -z "${VERSIONFILE}" ]
then
   VERSIONFILE="`get_header_from_name "${NAME}"`"
fi

if [ -z "${VERSIONNAME}" ]
then
   VERSIONNAME="`get_versionname_from_project "${PROJECT}"`"
fi

if [ -f VERSION ]
then
   VERSION="`head -1 VERSION`"
else
   VERSION="`get_project_version "${VERSIONFILE}" "${VERSIONNAME}"`"
   if [ -z "${VERSION}" ]
   then
      VERSION="`get_project_version "src/version.h" "${VERSIONNAME}"`"
   fi
fi

# where homebrew grabs the archive off
ARCHIVE_URL="${ARCHIVE_URL:-https://github.com/${PUBLISHER}/${NAME}/archive/${VERSION}.tar.gz}"

# written into formula for homebrew, will be evaled
HOMEPAGE_URL="${HOMEPAGE_URL:-https://github.com/${PUBLISHER}/${NAME}}"


# --- HOMEBREW TAP ---
# Specify to where and under what name to publish via your brew tap
#
if [ -z "${PUBLISHER_TAP}" ]
then
   fail "you need to specify a publisher tap with --publisher-tap (hint: <mulle-kybernetik/software/>)"
fi

TAPS_LOCATION="${TAPS_LOCATION:-..}"

HOMEBREW_TAP="${HOMEBREW_TAP:-${TAPS_LOCATION}/homebrew-`basename -- ${PUBLISHER_TAP}`}"

RBFILE="${RBFILE:-${NAME}.rb}"

# --- GIT ---

#
# require PUBLISHER (and PUBLISHER_TAP) as command line parameter, so
# that forks don't have to edit this constantly
#
if [ -z "${PUBLISHER}" ]
then
   fail "You need to specify a publisher with --publisher (hint: https://github.com/<publisher>)"
fi

if [ -z "${VERSION}" ]
then
   fail "Could not figure out the version. (hint: check VERSIONNAME, VERSIONFILE)"
fi

# tag to tag your release
TAG="${TAG:-${TAG_PREFIX}${VERSION}}"

# git remote to push to, usually origin
ORIGIN="${ORIGIN:-origin}"

# git remote to push to, usually github, can be empty
GITHUB="${GITHUB:-github}"

# git branch to release to, source is always current
BRANCH="${BRANCH:-release}"


main()
{
   # do the release
   git_main "${BRANCH}" "${ORIGIN}" "${TAG}" "${GITHUB}" || exit 1

   # generate the formula and push it
   homebrew_main "${PROJECT}" \
                 "${NAME}" \
                 "${VERSION}" \
                 "${DEPENDENCIES}" \
                 "${BUILD_DEPENDENCIES}" \
                 "${HOMEPAGE_URL}" \
                 "${DESC}" \
                 "${ARCHIVE_URL}" \
                 "${HOMEBREW_TAP}" \
                 "${RBFILE}"
}

main "$@"
