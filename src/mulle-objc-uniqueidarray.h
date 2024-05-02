//
//  mulle_objc_uniqueidarray.h
//  mulle-objc-runtime-universe
//
//  Created by Nat! on 28.05.17
//  Copyright (c) 2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2017 Codeon GmbH.
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

#ifndef mulle_objc_uniqueidarray_h__
#define mulle_objc_uniqueidarray_h__

#include "mulle-objc-uniqueid.h"
#include "include.h"
#include <assert.h>


//
// _mulle_objc_uniqueidarray is sometimes sorted and sometimes not.
// categoryids and protocolids are sorted, but protocolclassids aren't
// So use search or find with caution...
//
struct _mulle_objc_uniqueidarray
{
   unsigned int            n;
   int                     autosort;        // this is a constant
   mulle_objc_uniqueid_t   entries[ 1];
};


// this is just a simple array of uniqueids
// it's always sorted for bsearching
// compose it singlethreaded and then use it immutable
static inline  struct _mulle_objc_uniqueidarray  *
   mulle_objc_alloc_uniqueidarray( unsigned int n,
                                   struct mulle_allocator *allocator)
{
   struct _mulle_objc_uniqueidarray   *array;
   size_t                             size;

   size  = sizeof( struct _mulle_objc_uniqueidarray) - sizeof( mulle_objc_uniqueid_t)
           + n * sizeof( mulle_objc_uniqueid_t);
   array = mulle_allocator_calloc( allocator, 1, size);
   return( array);
}


static inline void
   mulle_objc_uniqueidarray_free( struct _mulle_objc_uniqueidarray  *p,
                                   struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}

static inline void
   mulle_objc_uniqueidarray_abafree( struct _mulle_objc_uniqueidarray  *p,
                                     struct mulle_allocator *allocator)
{
   mulle_allocator_abafree( allocator, p);
}


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_uniqueidarray
   *_mulle_objc_uniqueidarray_by_adding_ids( struct _mulle_objc_uniqueidarray  *p,
                                             unsigned int m,
                                             mulle_objc_uniqueid_t *uniqueids,
                                             int sort,
                                             struct mulle_allocator *allocator);

static inline struct _mulle_objc_uniqueidarray
   *_mulle_objc_uniqueidarray_by_adding_uniqueidarray( struct _mulle_objc_uniqueidarray  *p,
                                                       struct _mulle_objc_uniqueidarray  *q,
                                                       int sort,
                                                       struct mulle_allocator *allocator)
{
   return( _mulle_objc_uniqueidarray_by_adding_ids( p, q->n, q->entries, sort, allocator));
}


static inline struct _mulle_objc_uniqueidarray
   *_mulle_objc_uniqueidarray_by_adding_id( struct _mulle_objc_uniqueidarray  *p,
                                            mulle_objc_uniqueid_t uniqueid,
                                            int sort,
                                            struct mulle_allocator *allocator)
{
   return( _mulle_objc_uniqueidarray_by_adding_ids( p, 1, &uniqueid, sort, allocator));
}


static inline int
   _mulle_objc_uniqueidarray_search( struct _mulle_objc_uniqueidarray  *buf,
                                     mulle_objc_uniqueid_t search)
{
   int   first;
   int   last;
   int   middle;

   assert( search != MULLE_OBJC_NO_UNIQUEID && search != MULLE_OBJC_INVALID_UNIQUEID);

   first  = 0;
   last   = buf->n - 1;  // unsigned not good (need extra if)
   middle = (first + last) / 2;

   while( first <= last)
   {
      if( buf->entries[ middle] <= search)
      {
         if( buf->entries[ middle] == search)
            return( 1);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( 0);
}



static inline int
   _mulle_objc_uniqueidarray_find( struct _mulle_objc_uniqueidarray  *array,
                                   mulle_objc_uniqueid_t search)
{
   mulle_objc_uniqueid_t   *sentinel;
   mulle_objc_uniqueid_t   *p;

   assert( search != MULLE_OBJC_NO_UNIQUEID && search != MULLE_OBJC_INVALID_UNIQUEID);

   p        = &array->entries[ 0];
   sentinel = &p[ array->n];
   while( p < sentinel)
   {
      if( *p == search)
         return( 1);
   }
   return( 0);
}


#endif /* mulle_objc_uniqueidarray_h */
