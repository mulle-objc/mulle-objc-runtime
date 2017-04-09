//
//  mulle_objc_ivarlist.h
//  mulle-objc
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

//

#ifndef mulle_objc_ivarlist_h__
#define mulle_objc_ivarlist_h__

#include "mulle_objc_ivar.h"

#include <assert.h>


struct _mulle_objc_infraclass;


struct _mulle_objc_ivarlist
{
   unsigned int              n_ivars;  // must be #0 and same as struct _mulle_objc_methodlist

   struct _mulle_objc_ivar   ivars[ 1];
};


static inline size_t   mulle_objc_size_of_ivarlist( unsigned int n_ivars)
{
   return( sizeof( struct _mulle_objc_ivarlist) + (n_ivars - 1) * sizeof( struct _mulle_objc_ivar));
}


struct _mulle_objc_class;

struct _mulle_objc_ivar  *_mulle_objc_ivarlist_linear_search( struct _mulle_objc_ivarlist *list,
                                                              mulle_objc_ivarid_t ivarid);

static inline struct _mulle_objc_ivar  *_mulle_objc_ivarlist_binary_search( struct _mulle_objc_ivarlist *list,
                                                                            mulle_objc_ivarid_t ivarid)
{
   return( _mulle_objc_ivar_bsearch( list->ivars, list->n_ivars, ivarid));
}


static inline struct _mulle_objc_ivar  *_mulle_objc_ivarlist_search( struct _mulle_objc_ivarlist *list,
                                                                      mulle_objc_ivarid_t ivarid)

{
   if( list->n_ivars >= 14)
      return( _mulle_objc_ivarlist_binary_search( list, ivarid));
   return( _mulle_objc_ivarlist_linear_search( list, ivarid));
}


int   _mulle_objc_ivarlist_walk( struct _mulle_objc_ivarlist *list,
                                 int (*f)( struct _mulle_objc_ivar *, struct _mulle_objc_infraclass *, void *),
                                 struct _mulle_objc_infraclass *infra,
                                 void *userinfo);

void   mulle_objc_ivarlist_sort( struct _mulle_objc_ivarlist *list);

#pragma mark - API


static inline int   mulle_objc_ivarlist_walk( struct _mulle_objc_ivarlist *list,
                                              int (*f)( struct _mulle_objc_ivar *, struct _mulle_objc_infraclass *, void *),
                                              struct _mulle_objc_infraclass *infra,
                                              void *userinfo)
{
   if( ! list)
      return( -1);
   return( _mulle_objc_ivarlist_walk( list, f, infra, userinfo));
}

#endif /* defined(__MULLE_OBJC__mulle_objc_ivarlist__) */
