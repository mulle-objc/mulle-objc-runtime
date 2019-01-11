//
//  mulle_objc_class_convenience.h
//  mulle-objc-runtime
//
//  Created by Nat! on 10.07.16.
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
#ifndef mulle_objc_class_convenience_h__
#define mulle_objc_class_convenience_h__

#include "mulle-objc-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-universe.h"
#include "include.h"


// code not used in the universe, but useful for MulleObjC

#pragma mark - instance creation

MULLE_C_NON_NULL_RETURN
static inline struct mulle_allocator   *
    _mulle_objc_class_get_universe_allocator( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe     *universe;

   universe = _mulle_objc_class_get_universe( cls);
   return( _mulle_objc_universe_get_allocator( universe));
}


static inline struct _mulle_objc_object   *
    __mulle_objc_infraclass_alloc_instance_extra( struct _mulle_objc_infraclass *infra,
                                                  size_t extra,
                                                  struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader  *header;
   struct _mulle_objc_object        *obj;
   struct _mulle_objc_class         *cls;

   header = mulle_allocator_calloc( allocator, 1, _mulle_objc_infraclass_get_allocationsize( infra) + extra);
   obj    = _mulle_objc_objectheader_get_object( header);
   cls    = _mulle_objc_infraclass_as_class( infra);
   _mulle_objc_object_set_isa( obj, cls);
// only add this trace query for debugging because it slows things down!
#if DEBUG
   {
      extern void   _mulle_objc_class_trace_alloc_instance( struct _mulle_objc_class *cls,
                                                            void *obj,
                                                            size_t extra);
      struct _mulle_objc_universe   *universe;

      universe = _mulle_objc_class_get_universe( cls);
      if( universe->debug.trace.instance)
         _mulle_objc_class_trace_alloc_instance( cls, obj, extra);
   }
#endif
   return( obj);
}


static inline struct _mulle_objc_object *
    _mulle_objc_infraclass_alloc_instance_extra( struct _mulle_objc_infraclass *infra,
                                                 size_t extra)
{
   struct mulle_allocator   *allocator;

   allocator = _mulle_objc_infraclass_get_allocator( infra);
   return( __mulle_objc_infraclass_alloc_instance_extra( infra, extra, allocator));
}


// free with mulle_allocator_free
static inline struct _mulle_objc_object *
    _mulle_objc_infraclass_alloc_instance( struct _mulle_objc_infraclass *infra)
{
   struct mulle_allocator   *allocator;

   allocator = _mulle_objc_infraclass_get_allocator( infra);
   return( __mulle_objc_infraclass_alloc_instance_extra( infra, 0, allocator));
}



static inline struct _mulle_objc_object *
   mulle_objc_infraclass_alloc_instance( struct _mulle_objc_infraclass *infra)
{
   struct mulle_allocator   *allocator;

   if( ! infra)
      return( NULL);
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, 0));
}


static inline struct _mulle_objc_object   *
   mulle_objc_infraclass_alloc_instance_extra( struct _mulle_objc_infraclass *infra,
                                               size_t extra)
{
   if( ! infra)
      return( NULL);
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, extra));
}


static inline void *
   _mulle_objc_infraclass_get_classextra( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_infraclass_get_classpair( infra);
   return( _mulle_objc_classpair_get_classextra( pair));
}

#endif /* mulle_objc_class_universe_h */
