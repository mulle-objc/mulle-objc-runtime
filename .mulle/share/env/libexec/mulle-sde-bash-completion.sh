# shellcheck shell=bash

#
# install completion if not yet present.
# mulle-sde-bash-completion.sh will load the inferior completes
#
if [ "`type -t "_mulle_sde_complete"`" != "function" ]
then
   . "$(mulle-sde libexec-dir)/mulle-sde-bash-completion.sh"
fi
