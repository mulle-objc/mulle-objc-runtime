//
//  mulle_objc_fnv1.h
//  mulle-objc-runtime
//
//  Created by Nat! on 19.04.16.
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
#ifndef mulle_objc_fnv1_h__
# define mulle_objc_fnv1_h__

/* renamed to mulle_objc to keep symbols clean
 * It's an implementation of the [FNV1 hash](//en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash).
 * The 32 bit value is in no form and shape compatible with the
 * 64 bit value.
 */

# include <stdint.h>
# include <stddef.h>

#define MULLE_OBJC_FNV1_32_INIT    0x811c9dc5
#define MULLE_OBJC_FNV1_64_INIT    0xcbf29ce484222325ULL


uint32_t   _mulle_objc_chained_fnv1_32( void *buf, size_t len, uint32_t hash);
uint64_t   _mulle_objc_chained_fnv1_64( void *buf, size_t len, uint64_t hash);


static inline uint32_t   _mulle_objc_fnv1_32( void *buf, size_t len)
{
   return( _mulle_objc_chained_fnv1_32( buf, len, MULLE_OBJC_FNV1_32_INIT));
}


static inline uint64_t   _mulle_objc_fnv1_64( void *buf, size_t len)
{
   return( _mulle_objc_chained_fnv1_64( buf, len, MULLE_OBJC_FNV1_64_INIT));
}


static inline uintptr_t   _mulle_objc_fnv1( void *buf, size_t len)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_objc_fnv1_32( buf, len));
   return( (uintptr_t) _mulle_objc_fnv1_64( buf, len));
}


static inline uintptr_t   _mulle_objc_chained_fnv1( void *buf,
                                                   size_t len,
                                                   uintptr_t hash)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_objc_chained_fnv1_32( buf, len, (uint32_t) hash));
   return( (uintptr_t) _mulle_objc_chained_fnv1_64( buf, len, (uint64_t) hash));
}

#endif /* mulle _objc_fnv1_h */
