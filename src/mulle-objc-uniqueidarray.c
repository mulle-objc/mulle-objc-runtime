//
//  mulle_objc_uniqueidarray.c
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

#include "mulle-objc-uniqueidarray.h"

#include <stdlib.h>


static inline void
   _mulle_objc_uniqueidarray_sort( struct _mulle_objc_uniqueidarray  *p)
{
   mulle_qsort_r( p->entries,
                  p->n,
                  sizeof( mulle_objc_uniqueid_t),
                  _mulle_objc_uniqueid_compare_r,
                  NULL);
}


struct _mulle_objc_uniqueidarray
   *_mulle_objc_uniqueidarray_by_adding_ids( struct _mulle_objc_uniqueidarray  *p,
                                             unsigned int m,
                                             mulle_objc_uniqueid_t *uniqueids,
                                             int sort,
                                             struct mulle_allocator *allocator)
{
   struct _mulle_objc_uniqueidarray  *copy;
   unsigned int                       n;

   n = p->n + m;

   copy = mulle_objc_alloc_uniqueidarray( n, allocator);

   memcpy( copy->entries, p->entries, sizeof( mulle_objc_uniqueid_t) * p->n);
   memcpy( &copy->entries[ p->n], uniqueids, sizeof( mulle_objc_uniqueid_t) * m);

   copy->n = n;

   if( sort)
      _mulle_objc_uniqueidarray_sort( copy);

   return( copy);
}
