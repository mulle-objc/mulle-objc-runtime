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


MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
unsigned int   mulle_objc_taggedpointer_get_index( void *pointer)
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


MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
uintptr_t   mulle_objc_taggedpointer_get_unsigned_value( void *pointer)
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


MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
intptr_t   mulle_objc_taggedpointer_get_signed_value( void *pointer)
{
   intptr_t   value;

   value = (intptr_t) pointer;
   assert( value & mulle_objc_get_taggedpointer_mask());

   return( value >> mulle_objc_get_taggedpointer_shift());
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

   c.value  = value;
   c.bits  >>= 52 + 7;  // get exponent bits down
   c.bits   &= 0xF;
   return( c.bits == 0x8 || c.bits == 0x7);
}



MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
void   *mulle_objc_create_double_taggedpointer( double d, unsigned int index)
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
   c.bits  = (c.bits << 5) | (c.bits >> (64 - 5));
   return( (void *) (uintptr_t) ((c.bits & ~ (uint64_t) 0x7) | index));
}


#if 0
// on x86_64 the constants make this larger (0x26):
// mulle_objc_taggedpointer_get_double_value(void*):
//  mov    rax,rdi
//  and    rax,0xfffffffffffffff8
//  mov    rcx,rdi
//  or     rcx,0xe
//  test   dil,0x4
//  cmovne rcx,rax
//  rol    rcx,0x3b
//  movq   xmm0,rcx
//  ret
MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
double   mulle_objc_taggedpointer_get_double_value( void *pointer)
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

   // |eeeeeeem|mmmmmmmm|...|mmmmmmmm|mmmsxttt|
   c.bits >>= 3;                // shift three two bits into nothing
                                // upper three become zero (done if x==1)
   // |000eeeee|eemmmmmm|...|mmmmmmmm|mmmmmmsx|
   if( ! (c.bits & 0x1))
   {
      c.bits |= 0xE000000000000000LL;
   // |111eeeee|eemmmmmm|...|mmmmmmmm|mmmmmms0|
   }
   c.bits  = (c.bits >> 2) | c.bits << (64 - 2); // rotate back
   // |sxyyyeee|eeeemmmm|...|mmmmmmmm|mmmmmmmm|
   return( c.value);
}
#endif

//
// on x86_64 this "godbolts" into smaller (0x1F) and better looking code.
// Also better on ARM it seems...
//
// mulle_objc_taggedpointer_get_double_value(void*):
// mov    rdx,rdi
// mov    rax,rdi
// or     rdx,0x7
// and    rax,0xfffffffffffffff8
// and    edi,0x8
// cmove  rax,rdx
// ror    rax,0x5
// movq   xmm0,rax
// ret
//
MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
double   mulle_objc_taggedpointer_get_double_value( void *pointer)
{
   union
   {
      uint64_t   bits;
      double     value;
   } c;


   c.bits  = (uintptr_t) pointer;

   // |eeeeeeem|mmmmmmmm|...|mmmmmmmm|mmmsxttt|

   if( ! (c.bits & 0x8))
   {
      c.bits |= 0x7LL;
   // |eeeeeeem|mmmmmmmm|...|mmmmmmmm|mmms0111|
   }
   else
   {
      c.bits &= ~0x7LL;
      // |eeeeeeem|mmmmmmmm|...|mmmmmmmm|mmms1000|
   }

   c.bits  = (c.bits >> 5) | c.bits << (64 - 5); // rotate back
   // |sxyyyeee|eeeemmmm|...|mmmmmmmm|mmmmmmmm|
   return( c.value);
}



// ###
// # float
// # In 32 bit, this scheme should cover a lot of the range
// # 7.567e-10 - 6.97932e+09, which I would
// # assume is the majority of all real life float values
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

   c.value  = value;
   c.bits  >>= 23 + 5;  // get exponent bits down
   c.bits   &= 0x7;
   return( c.bits == 0x4 || c.bits == 0x3);
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
      c.bits  = (c.bits << 4) | (c.bits >> (32 - 4));
      return( (void *) (uintptr_t) ((c.bits & ~0x3) | index));
   }

   return( (void *) (uintptr_t) (((uint64_t) c.bits << 3) | index));
}


MULLE_C_STATIC_ALWAYS_INLINE
MULLE_C_CONST_RETURN
float   mulle_objc_taggedpointer_get_float_value( void *pointer)
{
   union
   {
      uint32_t   bits;
      float      value;
   } c;

   assert( sizeof( float) == sizeof( uint32_t));

   if( sizeof( uintptr_t) == sizeof( uint32_t))
   {
      c.bits  = (uint32_t) (uintptr_t) pointer;

      // |eeeeemmm|mmmmmmmm|mmmmmmmm|mmmmsxtt|

      if( ! (c.bits & 0x4))
      {
         c.bits |= 0x3UL;
         // |eeeeemmm|mmmmmmmm|mmmmmmmm|mmmms011|
      }
      else
      {
         c.bits &= ~0x3UL;
         // |eeeeemmm|mmmmmmmm|mmmmmmmm|mmmms100|
      }

      c.bits  = (c.bits >> 4) | c.bits << (32 - 4); // rotate back
      // |sxyyeeee|emmmmmmm|mmmmmmmm|mmmmmmmm|
      return( c.value);
   }

   // see|eeeee|emmmmmmm|mmmmmmmm|mmmmmmmmiii| our representation
   //    |seeeeeee|emmmmmmm|mmmmmmmm|mmmmmmmm| IEEE754 representation
   assert( (uintptr_t) pointer & mulle_objc_get_taggedpointer_mask());
   c.bits = (uint32_t) ((uintptr_t) pointer >> 3);
   return( c.value);
}

#endif
