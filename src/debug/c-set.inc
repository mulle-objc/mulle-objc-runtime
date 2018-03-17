//
//  c_set.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.05.16.
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
#ifndef MULLE_C11_NO_NOOB_WARNINGS
# define MULLE_C11_NO_NOOB_WARNINGS  1
#endif
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


// this is an older version of _mulle_pointerset

typedef struct
{
   unsigned int   n;
   void           **storage;
   int            sorted;
} c_set;


static inline void   c_set_init( c_set *set)
{
   memset( set, 0, sizeof( c_set));
}


static inline void   c_set_done( c_set *set)
{
   free( set->storage);
}


static inline void   c_set_add( c_set *set, void *p)
{
   set->storage = realloc( set->storage, sizeof( *set->storage) * (set->n + 1));
   if( ! set->storage)
      abort();
   set->storage[ set->n++] = p;
   set->sorted = 0;
}


static int  _pointer_compare( void *p_a, void *p_b)
{
   intptr_t   a;
   intptr_t   b;

   a = *(intptr_t *) p_a;
   b = *(intptr_t *) p_b;
   return( a != b ? (a < b ? -1 : 1) : 0);
}


static void   *voidptr_bsearch( void **buf,
                                unsigned int n,
                                void  *search)
{
   int    first;
   int    last;
   int    middle;
   void   *p;
   
   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;
   
   while( first <= last)
   {
      p = buf[ middle];
      if( (uintptr_t) p <= (uintptr_t) search)
      {
         if( (uintptr_t) p == (uintptr_t) search)
            return( p);
         
         first = middle + 1;
      }
      else
         last = middle - 1;
 
      middle = (first + last) / 2;
   }
   
   return( NULL);
}


static void   *c_set_member2( c_set *set, void *p)
{
   void  **result;
   
   if( ! set->n)
      return( NULL);
   
   if( ! set->sorted)
   {
      qsort( set->storage, set->n, sizeof( void *), (int (*)()) _pointer_compare);
      set->sorted = 1;
   }
   result = voidptr_bsearch( set->storage, set->n, p);
   return( result ? *result : NULL);
}


static inline void   *c_set_member( c_set *set, void *p)
{
   void   **q;
   void   **sentinel;

   if( set->n >= 2)
      return( c_set_member2( set, p));

   q        = set->storage;
   sentinel = &q[ set->n];

   while( q < sentinel)
      if( *q++ == p)
         return( p);
   return( NULL);
}

