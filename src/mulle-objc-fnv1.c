//
//  mulle_objc_fnv1.c
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

#include "mulle-objc-fnv1.h"


#define FNV1_32_PRIME   0x01000193


uint32_t   _mulle_objc_chained_fnv1_32( void *buf, size_t len, uint32_t hash)
{
   unsigned char   *s;
   unsigned char   *sentinel;

   s        = buf;
   sentinel = &s[ len];

   /*
    * FNV-1 hash each octet in the buffer
    */
   while( s < sentinel)
   {
      hash *= FNV1_32_PRIME;
      hash ^= (uint32_t) *s++;
   }

   return( hash);
}


#define FNV1_64_PRIME   0x0100000001b3ULL


uint64_t   _mulle_objc_chained_fnv1_64( void *buf, size_t len, uint64_t hash)
{
   unsigned char   *s;
   unsigned char   *sentinel;

   s        = buf;
   sentinel = &s[ len];

   /*
    * FNV-1 hash each octet in the buffer
    */
   while( s < sentinel)
   {
      hash *= FNV1_64_PRIME;
      hash ^= (uint64_t) *s++;
   }

   return( hash);
}


// Build it with:
//
// cc -o mulle_objc_fnv1 -DMAIN mulle_objc_fnv1.c
// to check if it produces same results as reference implementation
//
#ifdef MAIN

#include <stdio.h>
#include <string.h>


int   main( int argc, char * argv[])
{
   if( argc != 2)
      return( -1);

   printf( "0x%08x\n", (uint32_t) _mulle_objc_fnv1_32( argv[ 1], strlen( argv[ 1])));
   return 0;
}

#endif
