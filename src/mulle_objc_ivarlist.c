//
//  mulle_objc_ivarlist.c
//  mulle-objc-runtime
//
//  Created by Nat! on 13.08.15.
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
#pragma clang diagnostic ignored "-Wparentheses"

#include "mulle_objc_ivarlist.h"

#include <stdlib.h>
#include <assert.h>


struct _mulle_objc_ivar  *_mulle_objc_ivarlist_linear_search( struct _mulle_objc_ivarlist *list,
                                                              mulle_objc_ivarid_t ivarid)
{
   struct _mulle_objc_ivar   *sentinel;
   struct _mulle_objc_ivar   *p;

   assert( list);
   assert( ivarid != MULLE_OBJC_NO_IVARID && ivarid != MULLE_OBJC_INVALID_IVARID);

   p        = &list->ivars[ 0];
   sentinel = &p[ list->n_ivars];

   while( p < sentinel)
   {
      if( p->descriptor.ivarid == ivarid)
         return( p);
      ++p;
   }

   return( 0);
}


int   _mulle_objc_ivarlist_walk( struct _mulle_objc_ivarlist *list,
                                 int (*f)( struct _mulle_objc_ivar *, struct _mulle_objc_infraclass *, void *),
                                 struct _mulle_objc_infraclass *infra,
                                 void *userinfo)
{
   struct _mulle_objc_ivar   *sentinel;
   struct _mulle_objc_ivar   *p;
   int                       rval;

   assert( list);

   p        = &list->ivars[ 0];
   sentinel = &p[ list->n_ivars];

   while( p < sentinel)
   {
      if( rval = (*f)( p, infra, userinfo))
         return( rval);
      ++p;
   }

   return( 0);
}


void    mulle_objc_ivarlist_sort( struct _mulle_objc_ivarlist *list)
{
   if( list)
      mulle_objc_ivar_sort( list->ivars, list->n_ivars);
}
