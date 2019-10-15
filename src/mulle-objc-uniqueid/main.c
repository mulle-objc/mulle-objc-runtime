//
//  main.c
//  mulle-objc-runtime-uniqueid
//
//  Created by Nat! on 19.04.16.
//  Copyright (c) 2016 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#include "mulle-objc-uniqueid.h"
#include <ctype.h>
#ifdef _WIN32
# include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#pragma clang diagnostic ignored "-Wparentheses"

static void   print_uniqueid( char *s, size_t len,
                              unsigned long value,
                              char *prefix, char *suffix)
{
#ifdef _WIN32
   char   *buf = alloca( sizeof( char) * (len + 1));
#else
   char   buf[ len + 1];
#endif
   char   *s1, *s2;
   char   c;

   s1 = s;
   s2 = buf;
   while( c = *s1++)
      *s2++ = (char) toupper( c);
   *s2 = c;

   printf( "#define %s%s%s   MULLE_OBJC_METHODID( 0x%08lx)  // \"%s\"\n",
            prefix,
            buf,
            suffix,
            value,
            s);
}

//
// grep through all words and get a CSV list of the hashes:
//
// grep -h -o -R --include "*.m" --include "*.h" -E '\w+' src \
//    | sort \
//    | sort -u \
//    | CSV="" xargs dependency/bin/mulle-objc-uniqueid
//
// Not that useful, because grep doesn't grep selectors well
//
int   main( int argc, char *argv[])
{
   unsigned long   value;
   char            **sentinel;
   char            *csv;
   char            *prefix;
   char            *suffix;
   char            *s;
   size_t          len;

   if( argc < 2 || ! strlen( argv[ 1]))
   {
      fprintf( stderr, "Usage:\n   mulle-objc-uniqueid <string>*\n"
                       "    Generates @selector() values from strings."
                       "    Based on fnv1%s32 with shift %d\n"
                       "\n"
                       "Environment:\n"
                       "   CSV:     output hash;name\n"
                       "   PREFIX:  output a #define with prefix\n"
                       "   SUFFIX:  suffix for #define (only if prefix is defined)\n"
                       "\n",
                       MULLE_OBJC_UNIQUEHASH_ALGORITHM == MULLE_OBJC_UNIQUEHASH_FNV1A
                         ? "a" : "",
                       MULLE_OBJC_UNIQUEHASH_SHIFT);
      return( -1);
   }

   csv    = getenv( "CSV");
   prefix = getenv( "PREFIX");
   suffix = getenv( "SUFFIX");
   if( ! suffix)
      suffix = "_METHODID";

   sentinel = &argv[ argc];
   argv     = &argv[ 1];

   while( argv < sentinel)
   {
      s     = *argv++;
      len   = strlen( s);
      value = (unsigned long) mulle_objc_uniqueid_from_string( s);
      if( csv)
      {
         printf( "%08lx;%s\n", value, s);
         continue;
      }

      if( ! prefix)
      {
         printf( "%08lx\n", value);
         continue;
      }

      print_uniqueid( s, len, value, prefix, suffix);
   }

   return( 0);
}
