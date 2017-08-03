//
//  mulle_objc_method.c
//  mulle-objc-runtime
//
//  Created by Nat! on 17/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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
#include "mulle_objc_method.h"

#include "mulle_objc_universe.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#pragma mark - bsearch

struct _mulle_objc_method   *_mulle_objc_method_bsearch( struct _mulle_objc_method *buf,
                                                         unsigned int n,
                                                         mulle_objc_methodid_t search)
{
   int   first;
   int   last;
   int   middle;
   struct _mulle_objc_method   *p;

   assert( search != MULLE_OBJC_NO_METHODID && search != MULLE_OBJC_INVALID_METHODID);

   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->descriptor.methodid <= search)
      {
         if( p->descriptor.methodid == search)
            return( p);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( NULL);
}


#pragma mark - qsort

int  _mulle_objc_method_compare( struct _mulle_objc_method *a, struct _mulle_objc_method *b)
{
   mulle_objc_methodid_t   a_id;
   mulle_objc_methodid_t   b_id;

   a_id = a->descriptor.methodid;
   b_id = b->descriptor.methodid;
   if( a_id < b_id)
      return( -1);
   return( a_id != b_id);
}


void   mulle_objc_method_sort( struct _mulle_objc_method *methods,
                               unsigned int n)
{
   if( ! methods)
      return;

   qsort( methods, n, sizeof( struct _mulle_objc_method), (int (*)()) _mulle_objc_method_compare);
}


#pragma mark - methoddescriptor

int  mulle_objc_methoddescriptor_is_sane( struct _mulle_objc_methoddescriptor *p)
{
   if( ! p)
   {
      errno = EINVAL;
      return( 0);
   }

   if( p->methodid == MULLE_OBJC_NO_METHODID || p->methodid == MULLE_OBJC_INVALID_METHODID)
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! p->name || ! strlen( p->name))
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! p->signature || ! strlen( p->signature))
   {
      errno = EINVAL;
      return( 0);
   }

   // costly, but just print to keep API stable, don't throw
#if DEBUG
   {
      mulle_objc_methodid_t   correct;

      correct = mulle_objc_methodid_from_string( p->name);
      if( correct != p->methodid)
         fprintf( stderr, "mulle_objc_universe warning: \"%s\" should have methodid %08x but has methodid %08x\n", p->name, correct, p->methodid);
   }
#endif
   return( 1);
}
