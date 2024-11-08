//
//  mulle_objc_atomicpointer.h
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
#ifndef mulle_objc_atomicpointer_h__
#define mulle_objc_atomicpointer_h__


#include "include.h"

#include <stdio.h>


struct _mulle_objc_class;
struct _mulle_objc_object;
struct _mulle_objc_uniqueidarray;


union _mulle_objc_atomicclasspointer_t
{
   struct _mulle_objc_class    *cls;      // dont read, except when debugging
   mulle_atomic_pointer_t      pointer;
};


union _mulle_objc_atomicobjectpointer_t
{
   struct _mulle_objc_object    *object;      // dont read, except when debugging
   mulle_atomic_pointer_t       pointer;
};



union _mulle_objc_uniqueidarraypointer_t
{
   struct _mulle_objc_uniqueidarray  *array;      // dont read, except when debugging
   mulle_atomic_pointer_t            pointer;
};


// inline coz I am lazy
// https://stackoverflow.com/questions/2741683/how-to-format-a-function-pointer
// buf must be s_mulle_objc_sprintf_functionpointer_buffer



#define s_mulle_objc_sprintf_functionpointer_buffer (2 + sizeof( mulle_functionpointer_t) * 2 + 1)

//
// TODO: would be nice to skip empty leading zeroes
// Why is this inline ?
//
static inline void   mulle_objc_sprintf_functionpointer( char *buf,
                                                         mulle_functionpointer_t fp)
{
   uint8_t   *p;
   uint8_t   *sentinel;

   sprintf( buf, "0x");
#if __BIG_ENDIAN__
   p        = (uint8_t *) &fp;
   sentinel = &p[ sizeof( fp)];
   while( p < sentinel)
   {
      buf = &buf[ 2];
      sprintf( buf, "%02x", *p++);
   }
#else
   sentinel = (uint8_t *) &fp;
   p        = &sentinel[ sizeof( fp)];
   while( p > sentinel)
   {
      buf = &buf[ 2];
      sprintf( buf, "%02x", *--p);
   }
#endif
}

#endif
