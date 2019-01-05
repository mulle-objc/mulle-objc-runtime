[ -z "${MULLE_VIRTUAL_ROOT}" -o -z "${MULLE_UNAME}"  ] && \
   echo "Your script needs to setup MULLE_VIRTUAL_ROOT \
and MULLE_UNAME properly" >&2  && exit 1

MULLE_ENV_SHARE_DIR="${MULLE_VIRTUAL_ROOT}/.mulle/share/env"
MULLE_ENV_ETC_DIR="${MULLE_VIRTUAL_ROOT}/.mulle/etc/env"
# Top/down order of inclusion. Left overrides right if present.
# Keep these files (except environment-custom.sh) clean off manual edits so
# that mulle-env can read and set environment variables.
#
# .mulle/etc/env                        | .mulle/share/env
# --------------------------------------|--------------------
#                                       | environment-plugin.sh
#                                       | environment-plugin-os-${MULLE_UNAME}.sh
#                                       | environment-project.sh
#                                       | environment-extension.sh
# environment-global.sh                 |
# environment-os-${MULLE_UNAME}.sh      |
# environment-host-${MULLE_HOSTNAME}.sh |
# environment-user-${USER}.sh           |
# environment-custom.sh                 |
#

#
# The plugin file, if present is to be set by a mulle-env plugin
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-plugin.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-plugin.sh"
fi

#
# The plugin file, if present is to be set by a mulle-env plugin
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-plugin-os${MULLE_UNAME}.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-plugin-os${MULLE_UNAME}.sh"
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
# The extension file, if present is to be set by mulle-sde extensions.
#
if [ -f "${MULLE_ENV_SHARE_DIR}/environment-extension.sh" ]
then
   . "${MULLE_ENV_SHARE_DIR}/environment-extension.sh"
fi

#
# Global user settings
#
if [ -f "${MULLE_ENV_ETC_DIR}/environment-global.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-global.sh"
fi

#
# Load in some user modifications depending on os, hostname, username.
#
if [ -f "${MULLE_ENV_ETC_DIR}/environment-host-${MULLE_HOSTNAME}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-host-${MULLE_HOSTNAME}.sh"
fi

if [ -f "${MULLE_ENV_ETC_DIR}/environment-os-${MULLE_UNAME}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-os-${MULLE_UNAME}.sh"
fi

if [ -f "${MULLE_ENV_ETC_DIR}/environment-user-${USER}.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-user-${USER}.sh"
fi

#
# For more complex edits, that don't work with the cmdline tool
#
if [ -f "${MULLE_ENV_ETC_DIR}/environment-custom.sh" ]
then
   . "${MULLE_ENV_ETC_DIR}/environment-custom.sh"
fi
unset MULLE_ENV_ETC_DIR
unset MULLE_ENV_SHARE_DIR
