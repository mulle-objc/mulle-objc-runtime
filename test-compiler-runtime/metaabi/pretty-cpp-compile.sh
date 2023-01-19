#! /usr/bin/env bash


# these are added to preprocessed file so we get less boring warnings
add_compilation_helpers()
{
   cat << EOF
#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wgnu-alignof-expression"
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"

extern void   *mulle_objc_object_call( void *obj, int sel, void *param);
int           printf( const char *, ...);
void          *memset( void *, int, unsigned long);
void          *memcpy( void *, const void *, unsigned long);
#define       assert( ...)
EOF
}


#
# Makes the preprocessor macros easier to debug.
#
preprocess_file()
{
   dependency_dir="`mulle-sde dependency-dir`" || exit 1

   for subdir in "/Debug/" "/Release/" "/"
   do
      include_dir="${dependency_dir}${subdir}include"
      if [ -e "${include_dir}" ]
      then
         break
      fi
   done

   mulle-clang -E \
               -fmacro-backtrace-limit=0 \
               -DMULLE_PRETTY_CPP_COMPILE \
               -DMULLE_TEST=1 \
               -DMULLE_INCLUDE_DYNAMIC=1 \
               -I"${include_dir}" \
               "${1:--}"
}


pretty_print_stdin()
{
   # https://astyle.sourceforge.net/
   # apt install astyle

   astyle --indent-preproc-define \
          --indent-preproc-block \
          --indent-col1-comments \
          --break-blocks \
          --pad-oper \
          --pad-comma \
          --delete-empty-lines \
          --align-pointer=name \
          --break-elseifs \
          --break-one-line-headers \
          --remove-braces \
          --attach-return-type \
          --convert-tabs \
          --break-after-logical \
          --max-code-length=132 \
          --mode=c \
          --pad-method-prefix \
          --pad-return-type \
          --align-method-colon \
          --indent=spaces=3 \
          --style=bsd
}


remove_cpp_lines_stdin()
{
   egrep -v '^#' 
}



compile_file()
{
   mulle-clang -x objective-c -c "$1"
}

ifile="${1%.[cm]}.i" 

if [ "$1" = "${ifile}" ]
then
   echo "Give me .c or .m only" >&2
   exit 1
fi

(
   add_compilation_helpers
   preprocess_file "$1" \
   | pretty_print_stdin \
   | remove_cpp_lines_stdin 
)  > "${ifile}" &&
compile_file "${ifile}"

