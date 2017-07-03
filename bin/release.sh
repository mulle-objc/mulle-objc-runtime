#! /usr/bin/env bash
#
#   Copyright (c) 2017 Nat! - Codeon GmbH
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
#
# For documentation and help see:
#    https://github.com/mulle-nat/mulle-homebrew
#
# Run this somewhat like this (for real: remove -n):
#   ./bin/release.sh -v -n --publisher mulle-nat --publisher-tap mulle-kybernetik/software/
#

#######
# If you are using mulle-build, you don't hafta change anything
#######

#
# Generate your `def install` `test do` lines here. echo them to stdout.
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
MULLE_HOMEBREW_VERSION="5.1.0"

if [ -z "`command -v mulle-homebrew-env`" ]
then
   cat <<EOF >&2
mulle-homebrew-env not found in PATH.
Visit the homepage for installation instructions:
   https://github.com/mulle-nat/mulle-homebrew
EOF
   exit 1
fi

INSTALLED_MULLE_HOMEBREW_VERSION="`mulle-homebrew-env version`" || exit 1
LIBEXEC_DIR="`mulle-homebrew-env libexec-path`" || exit 1

. "${LIBEXEC_DIR}/mulle-files.sh"       || exit 1
. "${LIBEXEC_DIR}/mulle-homebrew.sh"    || exit 1
. "${LIBEXEC_DIR}/mulle-git.sh"         || exit 1
. "${LIBEXEC_DIR}/mulle-version.sh"     || exit 1
. "${LIBEXEC_DIR}/mulle-environment.sh" || exit 1


main()
{
   if [ "${DO_GIT_RELEASE}" != "YES" -a "${DO_GENERATE_FORMULA}" != "YES" ]
   then
      fail "Nothing to do! bin/version-info.sh and bin/formula-info.sh are missing"
   fi

   if [ "${DO_GIT_RELEASE}" = "YES" ]
   then
     # do the release
      git_main "${BRANCH}" "${ORIGIN}" "${TAG}" "${GITHUB}" || exit 1
   fi

   if [ "${DO_GENERATE_FORMULA}" != "YES" ]
   then
       return
   fi

   if [ -z "${PUBLISHER}" ]
   then
      fail "You need to specify a publisher with --publisher (hint: https://github.com/<publisher>)"
   fi

   if [ -z "${PUBLISHER_TAP}" ]
   then
      fail "You need to specify a publisher tap with --tap (hint: <mulle-kybernetik/software>)"
   fi

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
