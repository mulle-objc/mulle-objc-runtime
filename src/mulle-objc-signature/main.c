//
//  main.c
//  mulle-objc-signature
//
//  Created by Nat! on 19.03.19
//  Copyright (c) 2019 Nat! - Mulle kybernetiK.
//  Copyright (c) 2019 Codeon GmbH.
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
#include <mulle-c11/mulle-c11.h>

#include "mulle-objc-signature.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>


int   main( int argc, char *argv[])
{
   char                        **sentinel;
   char                        *next;
   char                        *s;
   size_t                      len;
   struct mulle_objc_typeinfo  info;

   if( argc < 2 || ! strlen( argv[ 1]))
   {
      fprintf( stderr, "Usage:\n   mulle-objc-signature <string>*\n"
                       "   Parses runtime signature strings and emits\n"
                       "   them as CSV\n"
                       "\n");
      return( -1);
   }

   sentinel = &argv[ argc];
   argv     = &argv[ 1];

   while( argv < sentinel)
   {
      s = *argv++;
      while( next = mulle_objc_signature_supply_typeinfo( s, NULL, &info))
      {
         printf( "%.*s;", (int) (next - s), s);
         s = next;
      }
      printf( "%s\n", s);
   }

   return( 0);
}
