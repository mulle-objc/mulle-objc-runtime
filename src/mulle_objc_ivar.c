//
//  mulle_objc_ivar.c
//  mulle-objc
//
//  Created by Nat! on 11.08.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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

#include "mulle_objc_ivar.h"

#include <assert.h>
#include <stdlib.h>
#include <errno.h>


struct _mulle_objc_ivar   *_mulle_objc_ivar_bsearch( struct _mulle_objc_ivar *buf,
                                                     unsigned int n,
                                                     mulle_objc_ivarid_t search)
{
   int                       first;
   int                       last;
   int                       middle;
   struct _mulle_objc_ivar   *p;

   assert( search != MULLE_OBJC_NO_IVARID && search != MULLE_OBJC_INVALID_IVARID);

   first  = 0;
   last   = n - 1;  // unsigned not good (need extra if)
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->descriptor.ivarid <= search)
      {
         if( p->descriptor.ivarid == search)
            return( p);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( NULL);
}



#pragma mark - qsort

int   _mulle_objc_ivar_compare( struct _mulle_objc_ivar *a, struct _mulle_objc_ivar *b)
{
   mulle_objc_ivarid_t   a_id;
   mulle_objc_ivarid_t   b_id;

   a_id = a->descriptor.ivarid;
   b_id = b->descriptor.ivarid;
   if( a_id < b_id)
      return( -1);
   return( a_id != b_id);
}


void   mulle_objc_ivar_sort( struct _mulle_objc_ivar *ivars, unsigned int n)
{
   if( ! ivars)
      return;

   qsort( ivars, n, sizeof( struct _mulle_objc_ivar), (int(*)())  _mulle_objc_ivar_compare);
}
