//
//  mulle_objc_method.h
//  mulle-objc-runtime
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
#ifndef mulle_objc_method_h__
#define mulle_objc_method_h__

#include "include.h"

#include "mulle-objc-uniqueid.h"

#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>


// tough decision, but I type self as void
// So in C a function pointer can be > sizeof( void *), therefore one shold
// avoid putting this into a mulle_pointerarray for example but use a
// mulle_structarray. Of course this is only for the purest of thoughts. As
// soon as you are using `dlsym` you are resigned to the fact, that a function
// pointer must fit into void pointer.
//
typedef void   *(*mulle_objc_implementation_t)( void *,
                                                mulle_objc_methodid_t,
                                                void *);

//
// idea... add a bit to this _mulle_objc_descriptor, that the compiler
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


// this enum is inside ->bits, shifted << 16
enum _mulle_objc_methodfamily
{
   _mulle_objc_methodfamily_none        = 0,

   _mulle_objc_methodfamily_alloc       = 1,
   _mulle_objc_methodfamily_copy        = 2,
   _mulle_objc_methodfamily_init        = 3,
   _mulle_objc_methodfamily_mutablecopy = 4,
   _mulle_objc_methodfamily_new         = 5,

   _mulle_objc_methodfamily_autorelease = 6,
   _mulle_objc_methodfamily_dealloc     = 7,
   _mulle_objc_methodfamily_finalize    = 8,
   _mulle_objc_methodfamily_release     = 9,
   _mulle_objc_methodfamily_retain      = 10,
   _mulle_objc_methodfamily_retaincount = 11,
   _mulle_objc_methodfamily_self        = 12,
   _mulle_objc_methodfamily_initialize  = 13,

   _mulle_objc_methodfamily_perform     = 14,
   // methods returning non-void, no args
   _mulle_objc_methodfamily_getter      = 32,
   // methods starting with set[A-Z] returning void and one arg
   _mulle_objc_methodfamily_setter      = 33,
   _mulle_objc_methodfamily_adder       = 34,
   _mulle_objc_methodfamily_remover     = 35
};


enum
{
   _mulle_objc_method_type_equality_mask        = 0x0C,  // other bits can vary from class to class

   _mulle_objc_method_check_unexpected_override = 0x01,
   _mulle_objc_method_preload                   = 0x02,
   _mulle_objc_method_aam                       = 0x04,
   _mulle_objc_method_variadic                  = 0x08,
   _mulle_objc_method_guessed_signature         = 0x10,
   _mulle_objc_method_designated_initializer    = 0x20,


   // this "coverage" bit gets set, when a method has been searched and found.
   // Should setting the bit be atomic, though it will never be cleared ?
   // And it's the only bit in this enum, that is set at runtime, so does
   // it really need to be atomic ? I don't think so. Things would change
   // if there are 2 writable bits though.

   _mulle_objc_method_searched_and_found        = 0x80,

   // user mask (MSB is currently used by MulleThreadSafeObject to mark
   // methods which should not lock)
   _mulle_objc_method_user_mask                 = 0x000F800,
   // these can be set like this:
   // #pragma clang attribute push(__attribute__((annotate("objc_user_0"))), apply_to = objc_method)
   _mulle_objc_method_user_attribute_0          = 0x0000800,  // type equal check
   _mulle_objc_method_user_attribute_1          = 0x0001000,  // type equal check
   _mulle_objc_method_user_attribute_2          = 0x0002000,  // NOT type equal check
   _mulle_objc_method_user_attribute_3          = 0x0004000,  // NOT type equal check
   _mulle_objc_method_user_attribute_4          = 0x0008000,  // // NOT type equal check, used by #threadsafe

   _mulle_objc_methodfamily_mask                = 0x03F0000,
};


#define  _mulle_objc_methodfamily_shift   16
#define  _mulle_objc_method_user_shift    11

#define  _mulle_objc_method_bits_equality_mask     \
        (_mulle_objc_method_user_attribute_0       \
         |_mulle_objc_method_user_attribute_1      \
         |_mulle_objc_method_type_equality_mask)

// generally pass in
static inline int   _mulle_objc_method_bits_type_equal( uint32_t a, uint32_t b)
{
   // check user mask and important type bits, if differs fail early
   // method family is not important
   if( (a ^ b) & _mulle_objc_method_bits_equality_mask)
      return( 0);
   return( 1);
}


# pragma mark - descriptor

//
//
struct _mulle_objc_descriptor
{
   mulle_objc_methodid_t   methodid;   // alignment to signature unclear!
   char                    *signature; // mulle_objc_compat: signature < name
   char                    *name;
   uint32_t                bits;       // TODO: make this a mulle_atomic_pointer_t
};

#define MULLE_OBJC_INVALID_DESCRIPTOR   ((struct _mulle_objc_descriptor *) -1)


# pragma mark - method descriptor petty accessors


static inline mulle_objc_methodid_t
   _mulle_objc_descriptor_get_methodid( struct _mulle_objc_descriptor *desc)
{
   return( desc->methodid);
}


static inline char   *
   _mulle_objc_descriptor_get_name( struct _mulle_objc_descriptor *desc)
{
   return( desc->name);
}


static inline char   *
   _mulle_objc_descriptor_get_signature( struct _mulle_objc_descriptor *desc)
{
   return( desc->signature);
}


static inline int
   _mulle_objc_descriptor_get_bits( struct _mulle_objc_descriptor *desc)
{
   return( desc->bits);
}


static inline int
   _mulle_objc_descriptor_is_preload_method( struct _mulle_objc_descriptor *desc)
{
   return( desc->bits & _mulle_objc_method_preload);
}


static inline int
   _mulle_objc_descriptor_is_variadic( struct _mulle_objc_descriptor *desc)
{
   return( desc->bits & _mulle_objc_method_variadic);
}


static inline int
   _mulle_objc_descriptor_is_hidden_override_fatal( struct _mulle_objc_descriptor *desc)
{
   return( desc->bits & _mulle_objc_method_check_unexpected_override);
}


static inline enum _mulle_objc_methodfamily
   _mulle_objc_descriptor_get_methodfamily( struct _mulle_objc_descriptor *desc)
{
   return( (unsigned long) (desc->bits & _mulle_objc_methodfamily_mask) >> _mulle_objc_methodfamily_shift);
}



static inline int
   _mulle_objc_descriptor_get_user_bits( struct _mulle_objc_descriptor *desc)
{
   return( (unsigned long) (desc->bits & _mulle_objc_method_user_mask) >> _mulle_objc_method_user_shift);
}


static inline int
   _mulle_objc_descriptor_set_user_bits( struct _mulle_objc_descriptor *desc, int bits)
{
   assert( bits >= 0 && bits < 0x20);
   return( (desc->bits & ~_mulle_objc_method_user_mask) | ((uint32_t) bits << _mulle_objc_method_user_shift));
}


static inline int
   _mulle_objc_descriptor_is_init_method( struct _mulle_objc_descriptor *desc)
{
   return( _mulle_objc_descriptor_get_methodfamily( desc) == _mulle_objc_methodfamily_init);
}


static inline int
   _mulle_objc_descriptor_is_setter_method( struct _mulle_objc_descriptor *desc)
{
   return( _mulle_objc_descriptor_get_methodfamily( desc) == _mulle_objc_methodfamily_setter);
}


static inline int
   _mulle_objc_descriptor_is_getter_method( struct _mulle_objc_descriptor *desc)
{
   return( _mulle_objc_descriptor_get_methodfamily( desc) == _mulle_objc_methodfamily_getter);
}


static inline int
   _mulle_objc_descriptor_is_adder_method( struct _mulle_objc_descriptor *desc)
{
   return( _mulle_objc_descriptor_get_methodfamily( desc) == _mulle_objc_methodfamily_adder);
}


static inline int
   _mulle_objc_descriptor_is_remover_method( struct _mulle_objc_descriptor *desc)
{
   return( _mulle_objc_descriptor_get_methodfamily( desc) == _mulle_objc_methodfamily_remover);
}


static inline int   _mulle_objc_descriptor_is_threadaffine( struct _mulle_objc_descriptor *desc)
{
   return( ! (desc->bits & _mulle_objc_method_user_attribute_4));
}


//
// this compares by name, signature and bits, for use in (mulle) qsort_r
//
MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_descriptor_pointer_compare_r( void **_a, void **_b, void *thunk);

# pragma mark - method descriptor consistency

// sets errno
MULLE_OBJC_RUNTIME_GLOBAL
int  mulle_objc_descriptor_is_sane( struct _mulle_objc_descriptor *p);



# pragma mark - method

//
// MEMO: if we move the implementation up, and put methods into the
// call cache, objc call speed would get punished by one indirection,
// but introspecting a method becomes MUCH faster...
//
struct _mulle_objc_method
{
   struct _mulle_objc_descriptor   descriptor;
   union
   {
      mulle_objc_implementation_t      value;
      mulle_atomic_functionpointer_t   implementation;
   };
};


# pragma mark - method petty accessors

static inline char  *
   _mulle_objc_method_get_name( struct _mulle_objc_method *method)
{
   return( method->descriptor.name);
}


static inline char *
   _mulle_objc_method_get_signature( struct _mulle_objc_method *method)
{
   return( method->descriptor.signature);
}


static inline mulle_objc_methodid_t
   _mulle_objc_method_get_methodid( struct _mulle_objc_method *method)
{
   return( method->descriptor.methodid);
}


static inline int
   _mulle_objc_method_get_bits( struct _mulle_objc_method *method)
{
   return( method->descriptor.bits);
}


static inline mulle_objc_implementation_t
   _mulle_objc_method_get_implementation( struct _mulle_objc_method *method)
{
   mulle_functionpointer_t  imp;

   imp = _mulle_atomic_functionpointer_read( &method->implementation);
   return( (mulle_objc_implementation_t) imp);
}


static inline void
   _mulle_objc_method_set_implementation( struct _mulle_objc_method *method,
                                          mulle_objc_implementation_t imp)
{
   _mulle_atomic_functionpointer_write( &method->implementation, (mulle_functionpointer_t) imp);
}


static inline struct _mulle_objc_descriptor *
   _mulle_objc_method_get_descriptor( struct _mulle_objc_method *method)
{
   return( &method->descriptor);
}


static inline mulle_objc_implementation_t
   _mulle_objc_method_cas_implementation( struct _mulle_objc_method *method,
                                          mulle_objc_implementation_t newimp,
                                          mulle_objc_implementation_t oldimp)

{
   mulle_functionpointer_t   f;

   f = __mulle_atomic_functionpointer_cas( &method->implementation,
                                           (mulle_functionpointer_t) newimp,
                                           (mulle_functionpointer_t) oldimp);
   return( (mulle_objc_implementation_t) f);
}


#pragma mark - bsearch

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_method   *
   _mulle_objc_method_bsearch( struct _mulle_objc_method *buf,
                               unsigned int n,
                               mulle_objc_methodid_t search);

static inline struct _mulle_objc_method   *
    mulle_objc_method_bsearch( struct _mulle_objc_method *buf,
                               unsigned int n,
                               mulle_objc_methodid_t search)
{
   if( ! buf)
      return( NULL);
   if( (int) n <= 0)
      return( NULL);
   return( _mulle_objc_method_bsearch( buf, n, search));
}


#pragma mark - qsort

// this compares by methodid only
MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_method_compare_r( void *a, void *b, void *thunk);


MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_method_sort( struct _mulle_objc_method *methods,
                               unsigned int n);


# pragma mark - method API

static inline char   *
   mulle_objc_method_get_name( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_name( method) : NULL);
}


static inline char *
   mulle_objc_method_get_signature( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_signature( method) : NULL);
}


static inline mulle_objc_methodid_t
   mulle_objc_method_get_methodid( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_methodid( method)
                  : MULLE_OBJC_NO_METHODID);
}


static inline mulle_objc_implementation_t
   mulle_objc_method_get_implementation( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_implementation( method) : 0);
}


static inline struct _mulle_objc_descriptor *
   mulle_objc_method_get_descriptor( struct _mulle_objc_method *method)
{
   return( method ? _mulle_objc_method_get_descriptor( method) : NULL);
}


static inline int   _mulle_objc_method_is_threadaffine( struct _mulle_objc_method *method)
{
   return( ! (_mulle_objc_method_get_bits( method) & _mulle_objc_method_user_attribute_4));
}


/* compatibility stuff for Foundation */

MULLE_OBJC_RUNTIME_GLOBAL
unsigned int   mulle_objc_count_selector_arguments( char *s);


static inline char  *mulle_objc_method_bit_utf8( unsigned long x)
{
   switch( x)
   {
   case _mulle_objc_method_check_unexpected_override : return( "check_unexpected_override");
   case _mulle_objc_method_preload                   : return( "preload");
   case _mulle_objc_method_aam                       : return( "aam");
   case _mulle_objc_method_variadic                  : return( "variadic");
   case _mulle_objc_method_guessed_signature         : return( "guessed_signature");
   case _mulle_objc_method_designated_initializer    : return( "designated_initializer");
   case _mulle_objc_method_searched_and_found        : return( "searched_and_found");

   case _mulle_objc_method_user_attribute_0          : return( "user_attribute_0");
   case _mulle_objc_method_user_attribute_1          : return( "user_attribute_1");
   case _mulle_objc_method_user_attribute_2          : return( "user_attribute_2");
   case _mulle_objc_method_user_attribute_3          : return( "user_attribute_3");
   case _mulle_objc_method_user_attribute_4          : return( "user_attribute_4(TAO)"); // used by #threadsafe
   }
   return( NULL);
}


static inline char  *mulle_objc_methodfamily_utf8( unsigned long x)
{
   switch( (x & _mulle_objc_methodfamily_mask) >> _mulle_objc_methodfamily_shift)
   {
   case _mulle_objc_methodfamily_none         : return( "none");
   case _mulle_objc_methodfamily_alloc        : return( "alloc");
   case _mulle_objc_methodfamily_copy         : return( "copy");
   case _mulle_objc_methodfamily_init         : return( "init");
   case _mulle_objc_methodfamily_mutablecopy  : return( "mutablecopy");
   case _mulle_objc_methodfamily_new          : return( "new");
   case _mulle_objc_methodfamily_autorelease  : return( "autorelease");
   case _mulle_objc_methodfamily_dealloc      : return( "dealloc");
   case _mulle_objc_methodfamily_finalize     : return( "finalize");
   case _mulle_objc_methodfamily_release      : return( "release");
   case _mulle_objc_methodfamily_retain       : return( "retain");
   case _mulle_objc_methodfamily_retaincount  : return( "retaincount");
   case _mulle_objc_methodfamily_self         : return( "self");
   case _mulle_objc_methodfamily_initialize   : return( "initialize");
   case _mulle_objc_methodfamily_perform      : return( "perform");
   case _mulle_objc_methodfamily_getter       : return( "getter");
   case _mulle_objc_methodfamily_setter       : return( "setter");
   case _mulle_objc_methodfamily_adder        : return( "adder");
   case _mulle_objc_methodfamily_remover      : return( "remover");
   default                                    : return( "???");
   }
}

#endif
