#! /usr/bin/env bash
#
#   Copyright (c) 2020 Nat! - Mulle kybernetiK
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

filesystem_task_run()
{
   log_entry "mulle-sde/c-cmake::filesystem_task_run" "$@"

   log_info "Reflecting ${C_MAGENTA}${C_BOLD}${PROJECT_NAME:-.}${C_INFO} filesystem"

   local rval 

   rval=0
   case "${MULLE_MATCH_TO_CMAKE_RUN}" in
      NO|DISABLE*|OFF)
      ;;

      *)
         exekutor mulle-match-to-cmake ${MULLE_TECHNICAL_FLAGS} "$@"  
         rval=$?
      ;;
   esac

   if [ $rval -ne 0 ]
   then
      log_error "mulle-match-to-cmake ${MULLE_TECHNICAL_FLAGS} $* failed ($rval)"
   fi

   local rval2

   rval2=0
   case "${MULLE_MATCH_TO_C_RUN}" in
      NO|DISABLE*|OFF)
      ;;

      *)
         exekutor mulle-match-to-c ${MULLE_TECHNICAL_FLAGS} "$@"
         rval2=$?
      ;;
   esac

   if [ $rval2 -ne 0 ]
   then
      log_error "mulle-match-to-c ${MULLE_TECHNICAL_FLAGS} $* failed ($rval2)"
   fi

   local rval3

   rval3=0
   case "${MULLE_PROJECT_CLIB_JSON_RUN}" in
      YES|ENABLE*|ON)
         case "${PROJECT_DIALECT}" in
            objc)
               exekutor mulle-project-clib-json ${MULLE_TECHNICAL_FLAGS} \
                                                -a src/reflect/objc-loader.inc \
                                                -o "clib.json"
               rval3=$?
            ;;

            *)
               exekutor mulle-project-clib-json ${MULLE_TECHNICAL_FLAGS} \
                                                -o "clib.json"
               rval3=$?
            ;;
         esac
      ;;
   esac

   if [ $rval3 -ne 0 ]
   then
      log_error "mulle-project-clib-json ${MULLE_TECHNICAL_FLAGS} failed ($rval3)"
   fi


   [ $rval -eq 0 -a $rval2 -eq 0 -o $rval3 -eq 0 ]
}
