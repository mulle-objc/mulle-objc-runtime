//
//  mulle_objc_object_convenience.h
//  mulle-objc-runtime
//
//  Created by Nat! on 11.03.15.
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
#ifndef mulle_objc_object_convenience_h__
#define mulle_objc_object_convenience_h__

#include "mulle-objc-object.h"

#include "mulle-objc-class.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-class-convenience.h"
#include "mulle-objc-call.h"

// methods on class shortcuts

static inline char   *_mulle_objc_object_get_isa_name( void *obj)
{
   return( _mulle_objc_class_get_name( _mulle_objc_object_get_isa( obj)));
}

static inline mulle_objc_implementation_t
    _mulle_objc_object_cacheonlylookup_implementation( void *obj,
                                                       mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_implementation_cacheonly( cls, methodid));
}


static inline mulle_objc_implementation_t
    _mulle_objc_object_lookup_implementation( void *obj,
                                              mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_implementation( cls, methodid));
}


static inline mulle_objc_implementation_t
    _mulle_objc_object_lookup_implementation_no_forward( void *obj,
                                                               mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_implementation_noforward( cls, methodid));
}


static inline struct _mulle_objc_method   *
    _mulle_objc_object_defaultsearch_method( void *obj,
                                             mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls  = _mulle_objc_object_get_isa( obj);
   return( mulle_objc_class_defaultsearch_method( cls, methodid));
}


MULLE_C_NONNULL_RETURN static inline struct mulle_allocator   *
    _mulle_objc_instance_get_allocator( void *obj)
{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_infraclass   *infra;

   cls   = _mulle_objc_object_get_isa( obj);
   infra = _mulle_objc_class_as_infraclass( cls);
   return( _mulle_objc_infraclass_get_allocator( infra));
}

#pragma mark - instance deletion

static inline void  __mulle_objc_instance_will_free( struct _mulle_objc_object *obj)
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


static inline void   __mulle_objc_instance_free( void *obj,
                                                 struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader  *header;

   __mulle_objc_instance_will_free( obj);

   header = _mulle_objc_object_get_objectheader( obj);
   mulle_allocator_free( allocator, header);
}


static inline void   _mulle_objc_instance_free( void *obj)
{
   struct mulle_allocator          *allocator;
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_infraclass   *infra;

   cls       = _mulle_objc_object_get_isa( obj);
   infra     = _mulle_objc_class_as_infraclass( cls);
   allocator = _mulle_objc_infraclass_get_allocator( infra);
   __mulle_objc_instance_free( obj, allocator);
}


static inline void   mulle_objc_instance_free( void *obj)
{
   if( ! obj)
      return;

   _mulle_objc_instance_free( obj);
}

#endif
