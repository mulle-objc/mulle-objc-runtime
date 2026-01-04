//
//  mulle-objc-class-impcache.c
//  mulle-objc-runtime
//
//  Copyright (c) 2021-2024 Nat! - Mulle kybernetiK.
//  Copyright (c) 2021-2024 Codeon GmbH.
//  All rights reserved.
//
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
#ifndef mulle_objc_class_impcache_h__
#define mulle_objc_class_impcache_h__

#include "include.h"

#include "mulle-objc-impcache.h"
#include "mulle-objc-class.h"



MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_NONNULL_FIRST
MULLE_C_NONNULL_RETURN
struct _mulle_objc_cache  *
   _mulle_objc_class_get_impcache_cache_atomic( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_cacheentry      *entries;
   struct _mulle_objc_cache           *cache;
   struct _mulle_objc_impcachepivot   *cachepivot;

   cachepivot = _mulle_objc_class_get_impcachepivot( cls);
   entries    = _mulle_objc_impcachepivot_get_entries_atomic( cachepivot);
   assert( entries);
   cache      = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   return( cache);
}


MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_NONNULL_FIRST
MULLE_C_NONNULL_RETURN
struct _mulle_objc_impcache  *
   _mulle_objc_class_get_impcache_atomic( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_cache      *cache;
   struct _mulle_objc_impcache   *impcache;

   cache    = _mulle_objc_class_get_impcache_cache_atomic( cls);
   impcache = _mulle_objc_cache_get_impcache_from_cache( cache);
   return( impcache);
}



MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_FIRST
void
    _mulle_objc_class_fill_impcache_method( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_method *method,
                                            mulle_objc_uniqueid_t uniqueid);

//
// pass uniqueid = MULLE_OBJC_NO_UNIQUEID, to invalidate all, otherwise we
// only invalidate all, if uniqueid is found
//
// returns 0x1 if invalidated, otherwise 0
//
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_FIRST
int   _mulle_objc_class_invalidate_impcacheentry( struct _mulle_objc_class *cls,
                                                  mulle_objc_methodid_t uniqueid);


#endif
