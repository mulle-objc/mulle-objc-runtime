[ -z "${MULLE_VIRTUAL_ROOT}" -o -z "${MULLE_UNAME}"  ] && \
   echo "Your script needs to setup MULLE_VIRTUAL_ROOT \
and MULLE_UNAME properly" >&2  && exit 1

MULLE_ENV_SHARE_DIR="${MULLE_VIRTUAL_ROOT}/.mulle/share/env"
MULLE_ENV_ETC_DIR="${MULLE_VIRTUAL_ROOT}/.mulle/etc/env"
# Top/down order of inclusion.
# Keep these files (except environment-custom.sh) clean off manual edits so
# that mulle-env can read and set environment variables.
#
# .mulle/etc/env                        | .mulle/share/env
# --------------------------------------|--------------------
#                                       | environment-plugin.sh
#                                       | environment-plugin-os-${MULLE_UNAME}.sh
# environment-project.sh                |
# environment-global.sh                 |
# environment-os-${MULLE_UNAME}.sh      |
# environment-host-${MULLE_HOSTNAME}.sh |
# environment-user-${MULLE_USERNAME}.sh |
# environment-custom.sh                 |
# environment-post-global.sh            |
#
scopes="s:plugin;5
s:plugin-os-${MULLE_UNAME};15
e:global;40
e:os-${MULLE_UNAME};60
e:host-${MULLE_HOSTNAME};80
e:user-${MULLE_USERNAME};100
e:custom;1000
e:post-global;2000"

if [ -f "${MULLE_ENV_ETC_DIR}/auxscope" ]
then
   auxscopes="`PATH=/bin:/usr/bin sed -e 's/^/e:/'  \
                    -e "s/\${MULLE_UNAME}/${MULLE_UNAME}/" \
                    -e "s/\${MULLE_HOSTNAME}/${MULLE_HOSTNAME}/" \
                    -e "s/\${MULLE_USERNAME}/${MULLE_USERNAME}/" \
                    "${MULLE_ENV_ETC_DIR}/auxscope"`"
   scopes="${scopes}"$'\n'"${auxscopes}"
fi

if [ -f "${MULLE_ENV_SHARE_DIR}/auxscope" ]
then
   auxscopes="`PATH=/bin:/usr/bin sed -e 's/^/s:/'  \
                    -e "s/\${MULLE_UNAME}/${MULLE_UNAME}/" \
                    -e "s/\${MULLE_HOSTNAME}/${MULLE_HOSTNAME}/" \
                    -e "s/\${MULLE_USERNAME}/${MULLE_USERNAME}/" \
                    "${MULLE_ENV_SHARE_DIR}/auxscope"`"
   scopes="${scopes}"$'\n'"${auxscopes}"
fi

#
# Load scopes according to priority now
#
for scope in `printf "%s\n" "${scopes}" \
              | PATH=/bin:/usr/bin sort -t';' -k2n -k1 \
              | PATH=/bin:/usr/bin sed -n -e 's/\(.*\);.*$/\1/p'`
do
   case "${scope}" in
      e:*)
         includefile="${MULLE_ENV_ETC_DIR}/environment-${scope#?:}.sh"
      ;;

      s:*)
         includefile="${MULLE_ENV_SHARE_DIR}/environment-${scope#?:}.sh"
      ;;

      *)
         continue;
      ;;
   esac

   if [ -f "${includefile}" ]
   then
      . "${includefile}"
   fi
done

unset scope
unset scopes
unset auxscopes
unset includefile

unset MULLE_ENV_ETC_DIR
unset MULLE_ENV_SHARE_DIR
