[ "${TRACE}" = "YES" ] && set -x  && : "$0" "$@"

[ -z "${MULLE_VIRTUAL_ROOT}" -o -z "${MULLE_UNAME}"  ] && \
   echo "Your script needs to setup MULLE_VIRTUAL_ROOT \
and MULLE_UNAME properly" >&2  && exit 1

case "${MULLE_UNAME}" in
   'mingw'*)
      MULLE_HOSTNAME="`PATH=/bin:/usr/bin hostname`" # don't export it
   ;;

   *)
      MULLE_HOSTNAME="`PATH=/bin:/usr/bin hostname -s`" # don't export it
   ;;
esac

MULLE_ENV_SHARE_DIR="${MULLE_VIRTUAL_ROOT}/.mulle-env/share"
MULLE_ENV_ETC_DIR="${MULLE_VIRTUAL_ROOT}/.mulle-env/etc"


# Top/down order of inclusion.
# Left overrides right if present.
#
# .mulle-env/etc                        | .mulle-env/share
# --------------------------------------|--------------------
#                                       | environment-aux.sh
#                                       | environment-project.sh
#                                       | environment-share.sh
# environment-global.sh                 |
# environment-os-${MULLE_UNAME}.sh      | environment-os-${MULLE_UNAME}.sh
# environment-host-${MULLE_HOSTNAME}.sh |
# environment-user-${USER}.sh           |
# custom-environment.sh                 |
#

#
# The aux file, if present is to be set by a mulle-env plugin
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-aux.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-aux.sh"
fi


#
# The project file, if present is to be set by mulle-sde init itself
# w/o extensions
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-project.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-project.sh"
fi

#
# The share file, if present is to be set by mulle-sde extensions.
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-share.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-share.sh"
fi


if [ -f "${MULLE_ENV_ETC_DIR}/environment-global.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-global.sh"
fi

#
# "os-" can be written by extensions also
#
if [ -f "${MULLE_ENV_ETC_DIR}/environment-os-${MULLE_UNAME}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-os-${MULLE_UNAME}.sh"
else
   if [ -f "${MULLE_ENV_SHARE_DIR}/environment-os-${MULLE_UNAME}.sh" ]
   then
      . "${MULLE_ENV_SHARE_DIR}/environment-os-${MULLE_UNAME}.sh"
   fi
fi

#
# Load in some modifications depending on hostname, username. These
# won't be provided by extensions or plugins.
#
# These settings could be "cased" in a single file, but it seems convenient.
# And more managable for mulle-env environment
#

if [ -f "${MULLE_ENV_ETC_DIR}/environment-host-${MULLE_HOSTNAME}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-host-${MULLE_HOSTNAME}.sh"
fi

if [ -f "${MULLE_ENV_ETC_DIR}/environment-user-${USER}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-user-${USER}.sh"
fi

#
# For more complex edits, that don't work with the cmdline tool
#
if [ -f "${MULLE_ENV_ETC_DIR}/custom-environment.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/custom-environment.sh"
fi

unset MULLE_ENV_ETC_DIR
unset MULLE_ENV_SHARE_DIR
unset MULLE_HOSTNAME
