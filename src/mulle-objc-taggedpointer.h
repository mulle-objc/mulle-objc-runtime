//
//  mulle_objc_taggedpointer.h
//  mulle-objc-runtime
//
//  Created by Nat! on 14.07.16.
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
#ifndef mulle_objc_taggedpointer_h__
#define mulle_objc_taggedpointer_h__

#include "include.h"  // for alignment mulle_objc_vararg.hcode

// malloc usually guarantees that alignment is suitable for any built in type
// (this could very well be 1 byte alignment on a hypothetical CPU).
// But objects may also appear temporarily on the stack(!) (-> cheatin strings)
//
// It is assumed that for objects and classes the address 32 bit or 64 bit
// even on the stack is at least "natural", which means that.
//
// in 32 bit, the bottom 2 bits are unused
// in 64 bit, the bottom 3 bits are unused
//
static inline unsigned int   mulle_objc_get_taggedpointer_mask( void)
{
   return( sizeof( uintptr_t) == sizeof( uint32_t) ? 0x3 : 0x7);
}


static inline unsigned int   mulle_objc_get_taggedpointer_shift( void)
{
   return( sizeof( uintptr_t) == sizeof( uint32_t) ? 2 : 3);
}


MULLE_C_ALWAYS_INLINE MULLE_C_CONST_RETURN
static inline unsigned int   mulle_objc_taggedpointer_get_index( void *pointer)
{
   uintptr_t   value;

   value = (uintptr_t) pointer;
   return( (unsigned int) (value & mulle_objc_get_taggedpointer_mask()));
}


// ###
// # NSUInteger
// ###
static inline uintptr_t   mulle_objc_taggedpointer_unsignedlostbitsmask( void)
{
   return( (UINTPTR_MAX & ~(UINTPTR_MAX >> mulle_objc_get_taggedpointer_shift())));
}


static inline int   mulle_objc_taggedpointer_is_valid_unsigned_value( uintptr_t value)
{
   return( ! (value & mulle_objc_taggedpointer_unsignedlostbitsmask()));
}


static inline void   *mulle_objc_create_unsigned_taggedpointer( uintptr_t value, unsigned int index)
{
   assert( index > 0 && index <= mulle_objc_get_taggedpointer_mask());

   // check that all shifted out bits are either all zeroes
   // or that in the signed case, the bits are all ones like the top bit
   assert( mulle_objc_taggedpointer_is_valid_unsigned_value( value));

   return( (void *) ((value << mulle_objc_get_taggedpointer_shift()) | index));
}


MULLE_C_ALWAYS_INLINE MULLE_C_CONST_RETURN
static inline uintptr_t   mulle_objc_taggedpointer_get_unsigned_value( void *pointer)
{
   uintptr_t   value;

   value = (uintptr_t) pointer;
   assert( value & mulle_objc_get_taggedpointer_mask());

   return( value >> mulle_objc_get_taggedpointer_shift());
}


// ###
// # NSInteger
// ###

static inline intptr_t   mulle_objc_taggedpointer_signedlostbitsmask( void)
{
   return( (intptr_t) (UINTPTR_MAX & ~(UINTPTR_MAX >> (mulle_objc_get_taggedpointer_shift() + 1))));
}


static inline int   mulle_objc_taggedpointer_is_valid_signed_value( intptr_t value)
{
   intptr_t  bits;

   bits = value & mulle_objc_taggedpointer_signedlostbitsmask();
   return( ! bits || bits == mulle_objc_taggedpointer_signedlostbitsmask());
}


static inline void   *mulle_objc_create_signed_taggedpointer( intptr_t value, unsigned int index)
{
   assert( index > 0 && index <= mulle_objc_get_taggedpointer_mask());

   // check that all shifted out bits are either all zeroes
   // or that in the signed case, the bits are all ones like the top bit
   assert( mulle_objc_taggedpointer_is_valid_signed_value( value));

   return( (void *) ((value << mulle_objc_get_taggedpointer_shift()) | index));
}


MULLE_C_ALWAYS_INLINE MULLE_C_CONST_RETURN
static inline intptr_t   mulle_objc_taggedpointer_get_signed_value( void *pointer)
{
   intptr_t   value;

   value = (intptr_t) pointer;
   assert( value & mulle_objc_get_taggedpointer_mask());

   return( value >> mulle_objc_get_taggedpointer_shift());
}


// ###
// # float
// ###

static inline int   mulle_objc_taggedpointer_is_valid_float_value( float value)
{
   union
   {
      uint32_t   bits;
      float      value;
   } c;

   assert( sizeof( float) == sizeof( uint32_t));

   // in 64 bit we can do all floats ez
   if( sizeof( uintptr_t) == sizeof( uint64_t))
      return( 1);

   c.value = value;
   c.bits  = (c.bits << 3) | (c.bits >> (32 - 3));
   return( (c.bits & 0x3) == 0x2);
}


static inline void   *mulle_objc_create_float_taggedpointer( float f, unsigned int index)
{
   union
   {
      uint32_t   bits;
      float      value;
   } c;

   assert( index > 0 && index <= mulle_objc_get_taggedpointer_mask());

   c.value = f;
   if( sizeof( uintptr_t) == sizeof( uint32_t))
   {
      c.bits  = (c.bits << 3) | (c.bits >> (32 - 3));
      assert( (c.bits & 0x3) == 0x2);
      return( (void *) (uintptr_t) (c.bits & ~0x3 | index));
   }
   return( (void *) (uintptr_t) (((uint64_t) c.bits << 3) | index));
}


//MULLE_C_ALWAYS_INLINE // removed for debugging
MULLE_C_CONST_RETURN
static inline float   mulle_objc_taggedpointer_get_float_value( void *pointer)
{
   union
   {
      uint32_t   bits;
      float      value;
   } c;

   assert( sizeof( float) == sizeof( uint32_t));

   if( sizeof( uintptr_t) == sizeof( uint32_t))
   {
      // |eeeeemmm|mmmmmmmm|mmmmmmmm|mmmmms11|  our representation
      // |s00eeeee|emmmmmmm|mmmmmmmm|mmmmmmmm|  IEEE754 representation
      //
      c.bits  = (uint32_t) (uintptr_t) pointer;
      assert( c.bits & mulle_objc_get_taggedpointer_mask());
      c.bits &= ~ (uint32_t) 0x3;  // mask off lower bits
      c.bits |= (uint32_t) 0x2;  // restore bit pattern
      c.bits  = (c.bits >> 3) | c.bits << (32 - 3); // rotate back
      return( c.value);
   }
   // s00|eeeee|emmmmmmm|mmmmmmmm|mmmmmmmm011| our representation
   //    |s00eeeee|emmmmmmm|mmmmmmmm|mmmmmmmm| IEEE754 representation
   assert( (uintptr_t) pointer & mulle_objc_get_taggedpointer_mask());
   c.bits = (uint32_t) ((uintptr_t) pointer >> 3);
   return( c.value);
}


// ###
// # double
// ###
static inline int   mulle_objc_taggedpointer_is_valid_double_value( double value)
{
   union
   {
      uint64_t   bits;
      double     value;
   } c;

   assert( sizeof( double) == sizeof( uint64_t));

   if( sizeof( uintptr_t) != sizeof( uint64_t))
      return( 0);

   c.value = value;
   c.bits  = (c.bits << 4) | (c.bits >> (64 - 4));

   return( (c.bits & 0x7) == 0x4);
}


MULLE_C_ALWAYS_INLINE MULLE_C_CONST_RETURN
static inline void   *mulle_objc_create_double_taggedpointer( double d, unsigned int index)
{
   union
   {
      uint64_t   bits;
      double     value;
   } c;

   assert( index > 0 && index <= mulle_objc_get_taggedpointer_mask());
   assert( sizeof( uintptr_t) == sizeof( uint64_t));
   assert( sizeof( uint64_t) == sizeof( double));

   c.value = d;
   c.bits  = (c.bits << 4) | (c.bits >> (64 - 4));
   assert( (c.bits & 0x7) == 0x4);
   return( (void *) (uintptr_t) (c.bits & ~ (uint64_t) 0x7 | index));
}


MULLE_C_ALWAYS_INLINE MULLE_C_CONST_RETURN
static inline double   mulle_objc_taggedpointer_get_double_value( void *pointer)
{
   union
   {
      uint64_t   bits;
      double     value;
   } c;

   assert( sizeof( double) == sizeof( uintptr_t));
   assert( sizeof( uintptr_t) == sizeof( uint64_t));

   c.bits  = (uintptr_t) pointer;
   assert( c.bits & mulle_objc_get_taggedpointer_mask());
   c.bits &= ~ (uintptr_t) 0x7; // mask off lower bits
   c.bits |= (uintptr_t) 0x4;  // restore bit pattern
   c.bits  = (c.bits >> 4) | (c.bits << (64 - 4)); // rotate back
   return( c.value);
}

#endif
