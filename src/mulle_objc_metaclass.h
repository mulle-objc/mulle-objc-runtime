//
//  mulle_objc_metaclass.h
//  mulle-objc
//
//  Created by Nat! on 17/04/07
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
#ifndef mulle_objc_metaclass_h__
#define mulle_objc_metaclass_h__

#include "mulle_objc_class_struct.h"
#include "mulle_objc_atomicpointer.h"
#include "mulle_objc_walktypes.h"
#include <assert.h>


enum _mulle_objc_metaclass_state
{
   MULLE_OBJC_META_CACHE_READY       = MULLE_OBJC_CACHE_READY,
   MULLE_OBJC_META_ALWAYS_EMPTY_CACHE = MULLE_OBJC_ALWAYS_EMPTY_CACHE,

   MULLE_OBJC_META_LOAD_SCHEDULED  = 0x10, // only if a class +load existed
   MULLE_OBJC_META_INITIALIZE_DONE = 0x20
};


struct _mulle_objc_metaclass
{
   struct _mulle_objc_class                 base;
};


static inline void   _mulle_objc_metaclass_plusinit( struct _mulle_objc_metaclass *meta,
                                              struct mulle_allocator *allocator)
{
}

static inline void   _mulle_objc_metaclass_plusdone( struct _mulle_objc_metaclass *meta)
{
}


static inline struct _mulle_objc_class   *_mulle_objc_metaclass_as_class( struct _mulle_objc_metaclass *meta)
{
   return( &meta->base);
}


static inline struct _mulle_objc_metaclass   *_mulle_objc_class_as_metaclass( struct _mulle_objc_class *cls)
{
   assert( _mulle_objc_class_is_metaclass( cls));

   return( (struct _mulle_objc_metaclass *) cls);
}


#pragma mark - sanity check

int   mulle_objc_metaclass_is_sane( struct _mulle_objc_metaclass *meta);



# pragma mark - conveniences

static inline struct _mulle_objc_runtime   *_mulle_objc_metaclass_get_runtime( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.runtime);
}


static inline mulle_objc_classid_t   _mulle_objc_metaclass_get_classid( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.classid);
}


static inline char   *_mulle_objc_metaclass_get_name( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.name);
}


static inline struct _mulle_objc_metaclass   *_mulle_objc_metaclass_get_superclass( struct _mulle_objc_metaclass *meta)
{
   return( (struct _mulle_objc_metaclass *) meta->base.superclass);
}


static inline unsigned int   _mulle_objc_metaclass_get_inheritance( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.inheritance);
}


static inline void   _mulle_objc_metaclass_set_inheritance( struct _mulle_objc_metaclass *meta, unsigned int inheritance)
{
   assert( (unsigned short) inheritance == inheritance);

   meta->base.inheritance = (unsigned short) inheritance;
}


static inline int   _mulle_objc_metaclass_set_state_bit( struct _mulle_objc_metaclass *meta,
                                                         enum _mulle_objc_metaclass_state bit)
{
   return( _mulle_objc_class_set_state_bit( &meta->base, (unsigned int) bit));
}


static inline unsigned int   _mulle_objc_metaclass_get_state_bit( struct _mulle_objc_metaclass *meta,
                                                                 enum _mulle_objc_metaclass_state bit)
{
   return( _mulle_objc_class_get_state_bit( &meta->base, (unsigned int) bit));
}


static inline void   mulle_objc_metaclass_unfailing_add_methodlist( struct _mulle_objc_metaclass *meta,
                                                                    struct _mulle_objc_methodlist *list)
{
   extern void   mulle_objc_class_unfailing_add_methodlist( struct _mulle_objc_class *cls,
                                                            struct _mulle_objc_methodlist *list);

   mulle_objc_class_unfailing_add_methodlist( &meta->base, list);
}


static inline struct _mulle_objc_method  *
    mulle_objc_metaclass_search_method( struct _mulle_objc_metaclass *meta,
                                         mulle_objc_methodid_t methodid)
{
   extern struct _mulle_objc_method   *mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                                                       mulle_objc_methodid_t methodid);

   return( mulle_objc_class_search_method( &meta->base, methodid));
}


mulle_objc_walkcommand_t
   mulle_objc_metaclass_walk( struct _mulle_objc_metaclass   *meta,
                               enum mulle_objc_walkpointertype_t  type,
                               mulle_objc_walkcallback_t   callback,
                               void *parent,
                               void *userinfo);

#endif /* mulle_objc_metaclass_h */
