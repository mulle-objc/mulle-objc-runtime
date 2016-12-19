//
//  mulle_objc_method.h
//  mulle-objc
//
//  Created by Nat! on 16/11/14.
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
//

#ifndef mulle_objc_method_h__
#define mulle_objc_method_h__

#include "mulle_objc_uniqueid.h"

#include <mulle_allocator/mulle_allocator.h>
#include <mulle_thread/mulle_thread.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>


// tough decision, but I type self as void
typedef void    *(*mulle_objc_methodimplementation_t)( void *, mulle_objc_methodid_t, void *);

// #define ? An enum is wrong when sizeof( int) != sizeof( uintptr_t)
//
// f.e. create finalize selector using `mulle-objc-uniqueid`
//
#define MULLE_OBJC_ALLOC_METHODID        MULLE_OBJC_METHODID( 0x1e16cffc)
#define MULLE_OBJC_AUTORELEASE_METHODID  MULLE_OBJC_METHODID( 0xf958ae7b)
#define MULLE_OBJC_CLASS_METHODID        MULLE_OBJC_METHODID( 0x5e8533db)
#define MULLE_OBJC_COPY_METHODID         MULLE_OBJC_METHODID( 0xdfc5a45a)
#define MULLE_OBJC_DEALLOC_METHODID      MULLE_OBJC_METHODID( 0x3a3d966b)
#define MULLE_OBJC_FINALIZE_METHODID     MULLE_OBJC_METHODID( 0xc39d1bbf)
#define MULLE_OBJC_INITIALIZE_METHODID   MULLE_OBJC_METHODID( 0x6543f237)
#define MULLE_OBJC_INIT_METHODID         MULLE_OBJC_METHODID( 0x50c63a23)
#define MULLE_OBJC_INSTANTIATE_METHODID  MULLE_OBJC_METHODID( 0x30f80cb7)
#define MULLE_OBJC_LOAD_METHODID         MULLE_OBJC_METHODID( 0x1f2fdaed)
#define MULLE_OBJC_RELEASE_METHODID      MULLE_OBJC_METHODID( 0x8f63473c)  // release
#define MULLE_OBJC_RETAIN_METHODID       MULLE_OBJC_METHODID( 0xd2f2322a)  // retain

#define MULLE_OBJC_FORWARD_METHODID      MULLE_OBJC_METHODID( 0x3f134576)

//
// idea... add a bit to this _mulle_objc_methoddescriptor, that the compiler
// sets, if this method is expected to override a method (maybe differentiate
// between a class method and a protocol method to override)
//
// When the selector is used (brought into the cache), it will check if it
// actually overrides a selector. If it does, and the bit is clear, that is an
// error and we abort.
//
// For this to work, the compiler must always set
//       _mulle_objc_method_check_unexpected_override
//
// Another bit gives preferential treatment to that selector in the cache.
// F.e. objectAtIndex: should always be (relative to the mask) in the first
// cache slot
//
// ----
// preloaded methods are placed into empty caches
// immediately, receiving the coveted #1 spot (usually)
//
enum
{
   _mulle_objc_method_check_unexpected_override = 0x01,
   _mulle_objc_method_preload                   = 0x02,
   _mulle_objc_method_aam                       = 0x04,
   _mulle_objc_method_variadic                  = 0x08,
   
// this is from clang ObjCMethodFamilyAttr
//  enum FamilyKind {
//    OMF_None,
//    OMF_alloc,
//    OMF_copy,
//    OMF_init,
//    OMF_mutableCopy,
//    OMF_new
//  };
   _mulle_objc_method_family_kind_none        = (0 << 16),
   _mulle_objc_method_family_kind_alloc       = (1 << 16),
   _mulle_objc_method_family_kind_copy        = (2 << 16),
   _mulle_objc_method_family_kind_init        = (3 << 16),
   _mulle_objc_method_family_kind_mutablecopy = (4 << 16),
   _mulle_objc_method_family_kind_new         = (5 << 16)
};

# pragma mark -
# pragma mark methoddescriptor

struct _mulle_objc_methoddescriptor
{
   mulle_objc_methodid_t   methodid;
   char                    *name;
   char                    *signature;
   unsigned int            bits;  
};


# pragma mark -
# pragma mark method descriptor petty accessors

static inline char   *_mulle_objc_methoddescriptor_get_name( struct _mulle_objc_methoddescriptor *desc)
{
   return( desc->name);
}


static inline char   *_mulle_objc_methoddescriptor_get_signature( struct _mulle_objc_methoddescriptor *desc)
{
   return( desc->signature);
}


static inline int   _mulle_objc_methoddescriptor_is_preload_method( struct _mulle_objc_methoddescriptor *desc)
{
   return( desc->bits & _mulle_objc_method_preload);
}


static inline int   _mulle_objc_methoddescriptor_is_variadic( struct _mulle_objc_methoddescriptor *desc)
{
   return( desc->bits & _mulle_objc_method_variadic);
}


static inline int   _mulle_objc_methoddescriptor_is_hidden_override_fatal( struct _mulle_objc_methoddescriptor *desc)
{
   return( desc->bits & _mulle_objc_method_check_unexpected_override);
}


static inline int   _mulle_objc_methoddescriptor_is_init_method( struct _mulle_objc_methoddescriptor *desc)
{
   return( desc->bits & _mulle_objc_method_family_kind_init);
}


# pragma mark -
# pragma mark method descriptor consistency

int  mulle_objc_methoddescriptor_is_sane( struct _mulle_objc_methoddescriptor *p);




# pragma mark -
# pragma mark method

struct _mulle_objc_method
{
   struct _mulle_objc_methoddescriptor  descriptor;
   union
   {
      mulle_objc_methodimplementation_t    value;
      mulle_atomic_functionpointer_t       implementation;
   };
};


# pragma mark -
# pragma mark method petty accessors

static inline char   *_mulle_objc_method_get_name( struct _mulle_objc_method *method)
{
   return( method->descriptor.name);
}


static inline char   *_mulle_objc_method_get_signature( struct _mulle_objc_method *method)
{
   return( method->descriptor.signature);
}


static inline mulle_objc_methodid_t  _mulle_objc_method_get_id( struct _mulle_objc_method *method)
{
   return( method->descriptor.methodid);
}


static inline mulle_objc_methodimplementation_t
   _mulle_objc_method_get_implementation( struct _mulle_objc_method *method)
{
   return( (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_read( &method->implementation));
}


static inline struct _mulle_objc_methoddescriptor *
   _mulle_objc_method_get_methoddescriptor( struct _mulle_objc_method *method)
{
   return( &method->descriptor);
}


#pragma mark -
#pragma mark bsearch

struct _mulle_objc_method   *_mulle_objc_method_bsearch( struct _mulle_objc_method *buf,
                                                         unsigned int n,
                                                         mulle_objc_methodid_t search);

static inline struct _mulle_objc_method   *mulle_objc_method_bsearch( struct _mulle_objc_method *buf,
                                                                      unsigned int n,
                                                                      mulle_objc_methodid_t search)
{
   assert( buf);
   if( ! buf || n > INT_MAX)
   {
      errno = EINVAL;
      return( (void *) INTPTR_MIN);
   }
   
   return( _mulle_objc_method_bsearch( buf, n, search));
}


#pragma mark -
#pragma mark qsort

int  _mulle_objc_method_compare( struct _mulle_objc_method *a, struct _mulle_objc_method *b);

void   mulle_objc_method_sort( struct _mulle_objc_method *methods,
                               unsigned int n);


# pragma mark -
# pragma mark method API

static inline char   *mulle_objc_method_get_name( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_name( method) : NULL);
}


static inline char   *mulle_objc_method_get_signature( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_signature( method) : NULL);
}


static inline mulle_objc_methodid_t  mulle_objc_method_get_id( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_id( method) : MULLE_OBJC_NO_METHODID);
}


static inline mulle_objc_methodimplementation_t
mulle_objc_method_get_implementation( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_implementation( method) : 0);
}


static inline struct _mulle_objc_methoddescriptor *
mulle_objc_method_get_methoddescriptor( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_methoddescriptor( method) : NULL);
}


#endif
