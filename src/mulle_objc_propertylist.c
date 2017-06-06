//
//  mulle_objc_propertylist.c
//  mulle-objc
//
//  Created by Nat! on 23.01.16.
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
#include "mulle_objc_propertylist.h"

#include "mulle_objc_class.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_callqueue.h"

#include <assert.h>
#include <stdlib.h>


int   _mulle_objc_propertylist_walk( struct _mulle_objc_propertylist *list,
                                    int (*f)( struct _mulle_objc_property *, struct _mulle_objc_infraclass *, void *),
                                    struct _mulle_objc_infraclass *infra,
                                    void *userinfo)
{
   struct _mulle_objc_property   *sentinel;
   struct _mulle_objc_property   *p;
   int                           rval;

   assert( list);

   p        = &list->properties[ 0];
   sentinel = &p[ list->n_properties];

   while( p < sentinel)
   {
      if( rval = (*f)( p, infra, userinfo))
         return( rval);
      ++p;
   }

   return( 0);
}


struct _mulle_objc_property  *_mulle_objc_propertylist_linear_search( struct _mulle_objc_propertylist *list,
                                                                   mulle_objc_propertyid_t propertyid)
{
   struct _mulle_objc_property   *sentinel;
   struct _mulle_objc_property   *p;

   assert( list);
   assert( propertyid != MULLE_OBJC_NO_PROPERTYID && propertyid != MULLE_OBJC_INVALID_PROPERTYID);

   p        = &list->properties[ 0];
   sentinel = &p[ list->n_properties];

   while( p < sentinel)
   {
      if( p->propertyid == propertyid)
         return( p);
      ++p;
   }

   return( 0);
}
