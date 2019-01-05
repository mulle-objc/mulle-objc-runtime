[ "${TRACE}" = "YES" ] && set -x  && : "$0" "$@"

[ -z "${MULLE_VIRTUAL_ROOT}" -o -z "${MULLE_UNAME}"  ] && \
   echo "Your script needs to setup MULLE_VIRTUAL_ROOT \
and MULLE_UNAME properly" >&2  && exit 1

HOSTNAME="`PATH=/bin:/usr/bin hostname -s`" # don't export it

MULLE_ENV_SHARE_DIR="${MULLE_VIRTUAL_ROOT}/.mulle-env/share"
MULLE_ENV_ETC_DIR="${MULLE_VIRTUAL_ROOT}/.mulle-env/etc"


#
# The aux file if present is to be set by mulle-sde extensions.
# The trick here is that mulle-env doesn't clobber this file
# when doing an init -f, which can be useful. There is no etc
# equivalent.
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-aux.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-aux.sh"
fi

#
# Default environment values set by plugins and extensions.
# The user should never edit them. He can override settings
# in etc.
#
if [ -f "${MULLE_ENV_ETC_DIR}/environment-global.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-global.sh"
else
   if [ -f "${MULLE_ENV_SHARE_DIR}/environment-global.sh" ]
   then
      . "${MULLE_ENV_SHARE_DIR}/environment-global.sh"
   fi
fi

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
# Load in some modifications depending on  hostname, username. These
# won't be provided by extensions or plugins.
#
# These settings could be "cased" in a single file, but it seems convenient.
# And more managable for mulle-env environment
#

if [ -f "${MULLE_ENV_ETC_DIR}/environment-host-${HOSTNAME}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-host-${HOSTNAME}.sh"
fi

if [ -f "${MULLE_ENV_ETC_DIR}/environment-user-${USER}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-user-${USER}.sh"
fi

#
# For more complex edits, that don't work with the cmdline tool
#
if [ -f "${MULLE_ENV_ETC_DIR}/environment-aux.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-aux.sh"
fi

unset MULLE_ENV_ETC_DIR
unset MULLE_ENV_SHARE_DIR
unset HOSTNAME
