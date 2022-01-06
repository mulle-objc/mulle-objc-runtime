#! /usr/bin/env bash
#
#   Copyright (c) 2018 Nat! - Mulle kybernetiK
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
[ "${TRACE}" = 'YES' -o "${MULLE_ENV_BASH_COMPLETION_SH_TRACE}" = 'YES' ] && set -x && : "$0" "$@"


#
# Old and stale :( :( :(
#
_mulle_env_style_complete()
{
   local cur=${COMP_WORDS[COMP_CWORD]}

   local toolstyles
   local envstyles

   toolstyles="${1:-`mulle-env toolstyles`}"
   envstyles="`mulle-env envstyles`"

   local ts
   local es
   local allstyles

   for ts in ${toolstyles}
   do
      for es in ${envstyles}
      do
         allstyles="${allstyles} ${ts}/${es}"
      done
   done

   COMPREPLY=( $( compgen -W "${allstyles}" -- "$cur" ) )
}


_mulle_env_complete()
{
   local cur=${COMP_WORDS[COMP_CWORD]}
   local prev=${COMP_WORDS[COMP_CWORD-1]}

   local list
   local i
   local context

   for i in "${COMP_WORDS[@]}"
   do
      case "$i" in
         environment|init|style|subenv|tool)
            context="$i"
         ;;
      esac
   done

   case "${context}" in
      tool)
         case "$prev" in
            remove)
               list="`mulle-env tool list`"
               COMPREPLY=( $( compgen -W "${list}" -- "$cur" ) )
            ;;

            list)
            ;;

            add)
            ;;

            *)
               COMPREPLY=( $( compgen -W "add remove list}" -- "$cur" ) )
            ;;
         esac
      ;;

      environment)
         case "$prev" in
            get|remove|set)
               list="`mulle-env -s environment list | sed 's/^\([^=]*\).*/\1/'`"
               COMPREPLY=( $( compgen -W "${list}" -- "$cur" ) )
            ;;

            list)
            ;;

            --scope)
               list="`mulle-env -s environment scope`"
               COMPREPLY=( $( compgen -W "${list}" -- "$cur" ) )
            ;;

            *)
               case "${cur}" in
                  -*)
                     COMPREPLY=( $( compgen -W "--global --hostname --os --scope --user" -- "$cur" ) )
                  ;;

                  *)
                     COMPREPLY=( $( compgen -W "get list set scope" -- "$cur" ) )
                  ;;
               esac
            ;;
         esac
      ;;

      subenv)
         COMPREPLY=( $( compgen -d -- "$cur" ) )
      ;;

      style)
         _mulle_env_style_complete
      ;;

      *)
         case "$prev" in
            -s|--style)
               _mulle_env_style_complete
            ;;

            -d|--directory)
               COMPREPLY=( $( compgen -d -- "$cur" ) )
            ;;

            *)
               case "$cur" in
                  -*)
                     COMPREPLY=( $( compgen -W "--style --directory" -- "$cur" ) )
                  ;;

                  *)
                     if [ "${context}" = "init" ]
                     then
                        COMPREPLY=( $( compgen -d -- "$cur" ) )
                     else
                        COMPREPLY=( $( compgen -W "environment init subenv style tool" -- $cur ) )
                     fi
                  ;;
               esac
            ;;
         esac
      ;;
   esac
}

complete -F _mulle_env_complete mulle-env

