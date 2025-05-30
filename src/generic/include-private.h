#ifndef mulle_objc_runtime_include_private_h__
#define mulle_objc_runtime_include_private_h__

#pragma clang diagnostic ignored "-Wparentheses"

/* This is a central include file to not expose includes to consumers of
   this library. It must not be imported by .h files, but by .c files
   only.
 */

#include "include.h"

/* Include the header file automatically generated by c-sourcetree-update.
   Here the prefix is harmless and serves disambiguation. If you have no
   sourcetree, then you don't need it.
 */

#include "_mulle-objc-runtime-include-private.h"

/* You can add some more include statements here */

#ifdef _WIN32
# include <malloc.h> //  for alloca
#endif

#endif
