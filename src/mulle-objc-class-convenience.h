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


// void as a return value is just easier to handle than
// struct _mulle_objc_object *

static inline void *
    __mulle_objc_infraclass_alloc_instance_extra( struct _mulle_objc_infraclass *infra,
                                                  size_t extra,
                                                  struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader   *header;
   void                              *alloc;
   struct _mulle_objc_object         *obj;
   struct _mulle_objc_class          *cls;
   size_t                            size;
   size_t                            metaextra;

   size = _mulle_objc_infraclass_get_allocationsize( infra) + extra;
   // if extra < 0, then overflow would happen undetected
   if( size <= extra)
      _mulle_allocator_fail( allocator, NULL, extra);

   // this is useful for debugging, but it's a small performance hit
#if DEBUG
   _mulle_atomic_pointer_increment( &infra->allocatedInstances);
#endif

   alloc     = _mulle_allocator_calloc( allocator, 1, size);
   cls       = _mulle_objc_infraclass_as_class( infra);
   metaextra = _mulle_objc_class_get_metaextrasize( cls);
   header    = _mulle_objc_alloc_get_objectheader( alloc, metaextra);
   _mulle_objc_objectheader_init( header, cls, metaextra, _mulle_objc_memory_is_zeroed);

   obj       = _mulle_objc_objectheader_get_object( header);

// only add this trace query for debugging because it slows things down!
#if DEBUG
   {
      void   _mulle_objc_infraclass_check_and_trace_alloc( struct _mulle_objc_infraclass *infra,
                                                           void *obj,
                                                           size_t extra);
      _mulle_objc_infraclass_check_and_trace_alloc( infra, obj, extra);
   }
#endif

   return( obj);
}


static inline void *
    __mulle_objc_infraclass_alloc_instance_extra_nonzeroed( struct _mulle_objc_infraclass *infra,
                                                            size_t extra,
                                                            struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader   *header;
   void                              *alloc;
   struct _mulle_objc_object         *obj;
   struct _mulle_objc_class          *cls;
   size_t                            size;
   size_t                            metaextra;

   size = _mulle_objc_infraclass_get_allocationsize( infra) + extra;
   // if extra < 0, then overflow would happen undetected
   if( size <= extra)
      _mulle_allocator_fail( allocator, NULL, extra);

   // this is useful for debugging, but it's a small performance hit
#if DEBUG
   _mulle_atomic_pointer_increment( &infra->allocatedInstances);
#endif

   alloc     = _mulle_allocator_malloc( allocator, size);
   cls       = _mulle_objc_infraclass_as_class( infra);
   metaextra = _mulle_objc_class_get_metaextrasize( cls);
   header    = _mulle_objc_alloc_get_objectheader( alloc, metaextra);
   _mulle_objc_objectheader_init( header, cls, metaextra, _mulle_objc_memory_is_not_zeroed);

   obj       = _mulle_objc_objectheader_get_object( header);

// only add this trace query for debugging because it slows things down!
#if DEBUG
   {
      void   _mulle_objc_infraclass_check_and_trace_alloc( struct _mulle_objc_infraclass *infra,
                                                           void *obj,
                                                           size_t extra);
      _mulle_objc_infraclass_check_and_trace_alloc( infra, obj, extra);
   }
#endif

   return( obj);
}


static inline void *
    _mulle_objc_infraclass_alloc_instance_extra_nonzeroed( struct _mulle_objc_infraclass *infra,
                                                           size_t extra)
{
   struct mulle_allocator   *allocator;

   allocator = _mulle_objc_infraclass_get_allocator( infra);
   return( __mulle_objc_infraclass_alloc_instance_extra_nonzeroed( infra, extra, allocator));
}


static inline void  __mulle_objc_instance_will_free( void *obj)
{
// too slow for non debug
#if DEBUG
   {
      struct _mulle_objc_universe    *universe;

      universe = _mulle_objc_object_get_universe( obj);
      if( universe->debug.trace.instance)
      {
         void   _mulle_objc_instance_trace_free( void *obj);

         _mulle_objc_instance_trace_free( obj);
      }
   }
#endif
}


static inline void
   __mulle_objc_infraclass_free_instance( struct _mulle_objc_infraclass *infra,
                                          void *obj,
                                          struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader   *header;
   struct _mulle_objc_class          *cls;
   void                              *alloc;

   __mulle_objc_instance_will_free( obj);

   header = _mulle_objc_object_get_objectheader( obj);

#if DEBUG
   _mulle_atomic_pointer_decrement( &infra->allocatedInstances);
   // malloc scribble will kill it though
   memset( obj, 0xad, _mulle_objc_class_get_instancesize( header->_isa));

   header->_isa = (void *) (intptr_t) 0xDEADDEADDEADDEAD;
   _mulle_atomic_pointer_nonatomic_write( &header->_retaincount_1, 0x0); // sic
#endif

   cls   = _mulle_objc_infraclass_as_class( infra);
   alloc = _mulle_objc_objectheader_get_alloc( header, cls->headerextrasize);
   _mulle_allocator_free( allocator, alloc);
}


static inline void
   _mulle_objc_infraclass_free_instance( struct _mulle_objc_infraclass *infra,
                                         void *obj)
{
   struct mulle_allocator   *allocator;

   allocator = _mulle_objc_infraclass_get_allocator( infra);
   __mulle_objc_infraclass_free_instance( infra, obj, allocator);
}



static inline void *
    _mulle_objc_infraclass_alloc_instance_extra( struct _mulle_objc_infraclass *infra,
                                                 size_t extra)
{
   struct mulle_allocator   *allocator;

   allocator = _mulle_objc_infraclass_get_allocator( infra);
   return( __mulle_objc_infraclass_alloc_instance_extra( infra, extra, allocator));
}


// free with mulle_allocator_free
static inline void *
    _mulle_objc_infraclass_alloc_instance( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, 0));
}


static inline void *
    _mulle_objc_infraclass_alloc_instance_zone( struct _mulle_objc_infraclass *infra,
                                                void *zone) // zone is unused
{
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, 0));
}


static inline void *
   mulle_objc_infraclass_alloc_instance( struct _mulle_objc_infraclass *infra)
{
   if( ! infra)
      return( NULL);
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, 0));
}


static inline void *
   mulle_objc_infraclass_allocwithzone_instance( struct _mulle_objc_infraclass *infra,
                                                 void *zone)
{
   if( ! infra)
      return( NULL);
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, 0));
}


static inline void *
   mulle_objc_infraclass_alloc_instance_extra( struct _mulle_objc_infraclass *infra,
                                               size_t extra)
{
   if( ! infra)
      return( NULL);
   return( _mulle_objc_infraclass_alloc_instance_extra( infra, extra));
}

// useless: just use a static variable in the @implementation
static inline void *
   _mulle_objc_infraclass_get_classextra( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_infraclass_get_classpair( infra);
   return( _mulle_objc_classpair_get_classextra( pair));
}

#endif /* mulle_objc_class_universe_h */
