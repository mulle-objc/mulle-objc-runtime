//
//  mulle_objc_property.c
//  mulle-objc-runtime
//
//  Created by Nat! on 05.03.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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
#pragma clang diagnostic ignored "-Wparentheses"

#include "mulle-objc-property.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


char   *_mulle_objc_property_signature_find_type( struct _mulle_objc_property *property, char type)
{
   char   *s;
   int    c;

   s = _mulle_objc_property_get_signature( property);
   while( c = *s++)
   {
      if( c == ',' && *s == type)
         return( s);
   }
   return( NULL);
}


// nice to travese signature for multiple types, ignoring others
char   *_mulle_objc_propertysignature_next_types( char *s, char *types)
{
   int    c;

   while( c = *s++)
      if( c == ',' && strchr( types, *s))
         return( s);
   return( NULL);
}


struct _mulle_objc_property   *_mulle_objc_property_bsearch( struct _mulle_objc_property *buf,
                                                             unsigned int n,
                                                            mulle_objc_propertyid_t search)
{
   int        first;
   int        last;
   int        middle;
   struct _mulle_objc_property   *p;

   assert( mulle_objc_uniqueid_is_sane( search));

   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->propertyid <= search)
      {
         if( p->propertyid == search)
            return( p);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( NULL);
}


int  _mulle_objc_property_compare( struct _mulle_objc_property *a,
                                   struct _mulle_objc_property *b)
{
   mulle_objc_propertyid_t   a_id;
   mulle_objc_propertyid_t   b_id;

   a_id = a->propertyid;
   b_id = b->propertyid;
   if( a_id < b_id)
      return( -1);
   return( a_id != b_id);
}


void   mulle_objc_property_sort( struct _mulle_objc_property *properties,
                                 unsigned int n)
{
   if( ! properties)
      return;

   qsort( properties, n, sizeof( struct _mulle_objc_property), (void *) _mulle_objc_property_compare);
}

