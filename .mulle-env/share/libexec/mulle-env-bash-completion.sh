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
[ "${TRACE}" = "YES" ] && set -x && : "$0" "$@"


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
         tool|environment|init)
            context="$i"
         ;;
      esac
   done

   case "${context}" in
      tool)
         case "$prev" in
            remove)
               list="`mulle-env -e tool list`"
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
            get|set)
               list="`mulle-env -s environment list | sed 's/^\([^=]*\).*/\1/'`"
               COMPREPLY=( $( compgen -W "${list}" -- "$cur" ) )
            ;;

            list)
            ;;

            *)
               COMPREPLY=( $( compgen -W "get list set" -- "$cur" ) )
            ;;
         esac
      ;;

      init)
      ;;

      *)
         case "$prev" in
            -s|--style)
               COMPREPLY=( $( compgen -W "\
none/wild none/inherit none/restrict none/relax none/tight \
minimal/wild minimal/inherit minimal/restrict minimal/relax minimal/tight \
developer/wild developer/inherit developer/restrict developer/relax developer/tight \
mulle/wild mulle/inherit mulle/restrict mulle/relax mulle/tight" -- "$cur" ) )
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
                     COMPREPLY=( $( compgen -W "environment init tool" -- $cur ) )
                  ;;
               esac
            ;;
         esac
      ;;
   esac
}

complete -F _mulle_env_complete mulle-env

