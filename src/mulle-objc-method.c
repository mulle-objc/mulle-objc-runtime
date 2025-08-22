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
#include "mulle-objc-method.h"

#include "mulle-objc-universe.h"
#include "mulle-objc-methodidconstants.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#pragma mark - bsearch

struct _mulle_objc_method   *_mulle_objc_method_bsearch( struct _mulle_objc_method *buf,
                                                         unsigned int n,
                                                         mulle_objc_methodid_t search)
{
   struct _mulle_objc_method   *p;
   int                         first;
   int                         last;
   int                         middle;

   assert( mulle_objc_uniqueid_is_sane( search));

   first  = 0;
   last   = n - 1;

   while( first <= last)
   {
      middle = (first + last) / 2;
      p      = &buf[ middle];
      if( p->descriptor.methodid <= search)
      {
         if( p->descriptor.methodid == search)
            return( p);

         first = middle + 1;
      }
      else
         last = middle - 1;
   }

   return( NULL);
}


#pragma mark - qsort

int  _mulle_objc_descriptor_pointer_compare_r( void **_a, void **_b, void *thunk)
{
   struct _mulle_objc_descriptor   *a = * (struct _mulle_objc_descriptor **) _a;
   struct _mulle_objc_descriptor   *b = * (struct _mulle_objc_descriptor **) _b;
   int                             cmp;
   long                            diff;

   MULLE_C_UNUSED( thunk);

   cmp = strcmp( a->name, b->name);
   if( cmp)
      return( cmp);

   cmp = strcmp( a->signature, b->signature);
   if( cmp)
      return( cmp);

   diff = (long) a->bits - (long) b->bits;
   return( diff < 0 ? -1 : ! ! diff);
}


int  _mulle_objc_method_compare_r( void *_a, void *_b, void *thunk)
{
   struct _mulle_objc_method   *a = _a;
   struct _mulle_objc_method   *b = _b;
   mulle_objc_methodid_t       a_id;
   mulle_objc_methodid_t       b_id;

   MULLE_C_UNUSED( thunk);

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

   mulle_qsort_r( methods,
                  n,
                  sizeof( struct _mulle_objc_method),
                  _mulle_objc_method_compare_r,
                  NULL);
}


#pragma mark - descriptor

int  mulle_objc_descriptor_is_sane( struct _mulle_objc_descriptor *p)
{
   if( ! p || ! p->name || ! p->signature || ! p->signature[ 0])
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! mulle_objc_uniqueid_is_sane_string( p->methodid, p->name))
      return( 0);

   return( 1);
}


#pragma mark - descriptor

unsigned int   mulle_objc_count_selector_arguments( char *s)
{
   unsigned int   n;
   char           c;
   if( ! s)
      return( (unsigned int) -1);

   n = 0;
   while( (c = *s++))
      if( c == ':')
         ++n;
   return( n);
}


