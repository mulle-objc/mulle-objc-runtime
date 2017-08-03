//
//  mulle_objc_uniqueid.c
//  mulle-objc-runtime
//
//  Created by Nat! on 05.02.16.
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
#include "mulle_objc_uniqueid.h"

#include "mulle_objc_fnv1.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>


mulle_objc_uniqueid_t  mulle_objc_uniqueid_from_string( char *s)
{
   mulle_objc_uniqueid_t  value;
   unsigned int           len;

   len = (unsigned int) strlen( s);
   if( ! s || ! len)
   {
      errno = EINVAL;
      return( MULLE_OBJC_INVALID_UNIQUEID);
   }
   value = _mulle_objc_fnv1_32( s, len);

   // make sure key is valid
   if( value == MULLE_OBJC_INVALID_UNIQUEID || value == MULLE_OBJC_NO_UNIQUEID)
      value = MULLE_OBJC_MIN_UNIQUEID;  // assumed to be unlikely

//   if( ! (value >> 16))
//      value |= 0x80000000;
//   if( ! (value & 0xFFFF))
//      value |= 0x1;

   return( value);
}


unsigned int   _mulle_objc_uniqueid_arraycount( mulle_objc_uniqueid_t *ids)
{
   unsigned int   n;

   n = 0;
   if( ids)
      while( *ids++)
         ++n;
   return( n);
}


int  _mulle_objc_uniqueid_qsortcompare( mulle_objc_uniqueid_t *a, mulle_objc_uniqueid_t *b)
{
   intptr_t   diff;

   diff = (intptr_t) *a - (intptr_t) *b;
   if( diff < 0)
      return( -1);
   return( ! ! diff);
}


#if 0
mulle_objc_uniqueid_t  mulle_objc_uniqueid_from_string( char *s)
{
   struct mulle_objc_md5       context;
   mulle_objc_uniqueid_t  value;
   unsigned char          digest[ MULLE_OBJC_MD5_DIGESTLENGTH];
   unsigned int           i;
   unsigned int           len;

   len = (unsigned int) strlen( s);
   if( ! s || ! len)
   {
      errno = EINVAL;
      return( MULLE_OBJC_INVALID_UNIQUEID);
   }

   mulle_objc_md5_init( &context);
   mulle_objc_md5_update( &context, (const unsigned char *) s, len);
   mulle_objc_md5_final( &context, digest);

   // non-optimal
   value = 0;
   for( i = 0; i < sizeof( mulle_objc_uniqueid_t); i++)
   {
      value <<= 8;
      value |= digest[ i];
   }
   return( value);
}
#endif

