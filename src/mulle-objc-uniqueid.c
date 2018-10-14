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
#include "mulle-objc-uniqueid.h"

#include "mulle-objc-fnv1.h"
#include "mulle-objc-fnv1a.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


mulle_objc_uniqueid_t   mulle_objc_uniqueid_from_string( char *s)
{
   mulle_objc_uniqueid_t  value;
   unsigned int           len;

   if( ! s)
   {
      errno = EINVAL;
      return( MULLE_OBJC_INVALID_UNIQUEID);
   }

   len = (unsigned int) strlen( s);
   if( ! len)
   {
      errno = EINVAL;
      return( MULLE_OBJC_INVALID_UNIQUEID);
   }

   if( MULLE_OBJC_UNIQUEHASH_ALGORITHM == MULLE_OBJC_UNIQUEHASH_FNV1A)
      value = _mulle_objc_fnv1a_32( s, len);
   else
      value = _mulle_objc_fnv1_32( s, len);

   //
   // fnv1 favors the last byte disproportionally, but that' usually just ':'
   // lets rotate it a bit, so the hash bit portions used for cache
   // access are better. This doesn't affect collisions at all as all bits
   // are preserved.
   //
   value = (value << MULLE_OBJC_UNIQUEHASH_SHIFT) | (value >> (32 - MULLE_OBJC_UNIQUEHASH_SHIFT));

   if( value == MULLE_OBJC_NO_UNIQUEID)
      value = MULLE_OBJC_INVALID_UNIQUEID;

   if( value == MULLE_OBJC_INVALID_UNIQUEID)
   {
      fprintf( stderr, "Congratulations, your string \"%s\" "
              "hashes badly (rare and precious, please tweet it @mulle_nat, then rename it).", s);
#ifdef DEBUG
      abort();
#endif
   }

   return( value);
}


int   mulle_objc_uniqueid_is_sane_string( mulle_objc_uniqueid_t uniqueid, char *s)
{
   if( ! mulle_objc_uniqueid_is_sane( uniqueid))
   {
      errno = EINVAL;
      return( 0);
   }

#if DEBUG
   {
      mulle_objc_uniqueid_t  correct;

      correct = mulle_objc_uniqueid_from_string( s);
      if( uniqueid != correct)
      {
         fprintf( stderr, "error: String \"%s\" should have "
                 "uniqueid %08x but has uniqueid %08x\n",
                 s, correct, uniqueid);
         errno = EINVAL;  // this is needed for tests
         return( 0);
      }
   }
#endif

   return( 1);
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

