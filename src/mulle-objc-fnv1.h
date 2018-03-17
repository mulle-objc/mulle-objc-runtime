//
//  mulle_objc_fnv1.h
//  mulle-objc-runtime
//
//  Created by Nat! on 19.04.16.

/***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *      chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!     :-)
 */

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
