#######
### none startup
#######
[ "${TRACE}" = 'YES' -o "${ENVIRONMENT_SH_TRACE}" = 'YES' ] && set -x  && : "$0" "$@"

#
# If mulle-env is broken, sometimes its nice just to source this file.
# If you're sourcing this manually on a regular basis, you're doing it wrong.
#
if [ -z "${MULLE_VIRTUAL_ROOT}" ]
then
   MULLE_VIRTUAL_ROOT="`PATH=/bin:/usr/bin pwd -P`"
   echo "Using ${MULLE_VIRTUAL_ROOT} as MULLE_VIRTUAL_ROOT for \
your convenience" >&2
fi

#
# now read in custom envionment (required)
#
. "${MULLE_VIRTUAL_ROOT}/.mulle/share/env/include-environment.sh"

#
# basic setup for interactive shells
#
case "${MULLE_SHELL_MODE}" in
   *INTERACTIVE*)
      #
      # Set PS1 so that we can see, that we are in a mulle-env
      #
      envname="`PATH=/bin:/usr/bin basename -- "${MULLE_VIRTUAL_ROOT}"`"

      case "${PS1}" in
         *\\h\[*)
         ;;

         *\\h*)
            PS1="$(sed 's/\\h/\\h\['${envname}'\]/' <<< '${PS1}' )"
         ;;

         *)
            PS1='\u@\h['${envname}'] \W$ '
         ;;
      esac
      export PS1

      unset envname

      # install cd catcher
      . "${MULLE_ENV_LIBEXEC_DIR}/mulle-env-cd.sh"
      unset MULLE_ENV_LIBEXEC_DIR

      # install mulle-env-reload

      alias mulle-env-reload='. "${MULLE_VIRTUAL_ROOT}/.mulle/share/env/include-environment.sh"'

      #
      # source in any bash completion files
      #
      DEFAULT_IFS="${IFS}"
      IFS=$'\n'
      # memo: nullglob not easily done on both bash and zsh
      for FILENAME in "${MULLE_VIRTUAL_ROOT}/.mulle/share/env/libexec"/*-bash-completion.sh
      do
         if [ -f "${FILENAME}" ]
         then
            . "${FILENAME}"
         fi
      done
      IFS="${DEFAULT_IFS}"

      unset DEFAULT_IFS
      unset FILENAME

      vardir="${MULLE_VIRTUAL_ROOT}/.mulle/var/${MULLE_HOSTNAME}"
      [ -d "${vardir}" ] || mkdir -p "${vardir}"

      HISTFILE="${vardir}/bash_history"
      export HISTFILE

      unset vardir

      #
      # show motd, if any
      #
      if [ -z "${NO_MOTD}" ]
      then
         if [ -f "${MULLE_VIRTUAL_ROOT}/.mulle/etc/env/motd" ]
         then
            cat "${MULLE_VIRTUAL_ROOT}/.mulle/etc/env/motd"
         else
            if [ -f "${MULLE_VIRTUAL_ROOT}/.mulle/share/env/motd" ]
            then
               cat "${MULLE_VIRTUAL_ROOT}/.mulle/share/env/motd"
            fi
         fi
      fi
   ;;
esac

# remove some uglies
unset NO_MOTD
unset TRACE

#######
### mulle startup
#######

case "${MULLE_SHELL_MODE}" in
   *INTERACTIVE*)
      if [ -z "" ]
      then
         alias craftorder="mulle-sde craftorder"
         alias clean="mulle-sde clean"
         alias craft="mulle-sde craft"
         alias dependency="mulle-sde dependency"
         alias environment="mulle-sde environment"
         alias extension="mulle-sde extension"
         alias fetch="mulle-sde fetch"
         alias show="mulle-sde show"
         alias list="mulle-sde list"
         alias library="mulle-sde library"
         alias log="mulle-sde log"
         alias match="mulle-sde match"
         alias monitor="mulle-sde monitor"
         alias reflect="mulle-sde reflect"
         alias patternfile="mulle-sde patternfile"
         alias subproject="mulle-sde subproject"
      fi

      if [ -z "" ]
      then
         alias c="mulle-sde craft"
         alias C="mulle-sde clean; mulle-sde craft"
         alias CC="mulle-sde clean all; mulle-sde craft"
         alias t="mulle-sde test rerun --serial"
         alias tt="mulle-sde test craft ; mulle-sde test rerun --serial"
         alias T="mulle-sde test craft ; mulle-sde test"
         alias TT="mulle-sde test clean all; mulle-sde test"
         alias r="mulle-sde reflect"
         alias l="mulle-sde list --files"
      fi
   ;;
esac
