//
//  mulle_objc_methodlist.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
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
#include "mulle-objc-methodlist.h"

#include "mulle-objc-class.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-callqueue.h"
#include "mulle-objc-methodidconstants.h"

#include <assert.h>
#include <stdlib.h>


mulle_objc_walkcommand_t   
   _mulle_objc_methodlist_walk( struct _mulle_objc_methodlist *list,
                                mulle_objc_method_walkcallback_t callback,
                                struct _mulle_objc_class *cls,
                                void *userinfo)
{
   struct _mulle_objc_method   *sentinel;
   struct _mulle_objc_method   *p;
   int                         rval;

   assert( list);

   p        = &list->methods[ 0];
   sentinel = &p[ list->n_methods];

   while( p < sentinel)
   {
      rval = (*callback)( p, list, cls, userinfo);
      if( rval)
         return( rval);
      ++p;
   }

   return( 0);
}


struct _mulle_objc_method  *
   _mulle_objc_methodlist_find( struct _mulle_objc_methodlist *list,
                                         mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *sentinel;
   struct _mulle_objc_method   *p;

   assert( list);
   assert( mulle_objc_uniqueid_is_sane( methodid));

   p        = &list->methods[ 0];
   sentinel = &p[ list->n_methods];

   while( p < sentinel)
   {
      if( _mulle_objc_method_get_methodid( p) == methodid)
         return( p);
      ++p;
   }

   return( 0);
}


struct _mulle_objc_method  *
   _mulle_objc_methodlist_linear_impsearch( struct _mulle_objc_methodlist *list,
                                            mulle_objc_implementation_t imp)
{
   struct _mulle_objc_method   *sentinel;
   struct _mulle_objc_method   *p;

   assert( list);
   assert( imp);

   p        = &list->methods[ 0];
   sentinel = &p[ list->n_methods];

   while( p < sentinel)
   {
      if( _mulle_objc_method_get_implementation( p) == imp)
         return( p);
      ++p;
   }

   return( 0);
}



void   mulle_objc_methodlist_sort( struct _mulle_objc_methodlist *list)
{
   if( list)
      mulle_objc_method_sort( list->methods, list->n_methods);
}



int
   mulle_objc_methodlist_add_load_to_callqueue( struct _mulle_objc_methodlist *list,
                                                struct _mulle_objc_metaclass *meta,
                                                struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_method      *method;
   mulle_objc_implementation_t    imp;

   assert( loads);

   // that's ok here, but not for _mulle_objc_methodlist_search_method
   if( ! list)
      return( 0);

   method = _mulle_objc_methodlist_search( list, MULLE_OBJC_LOAD_METHODID);
   if( method)
   {
      imp   = _mulle_objc_method_get_implementation( method);
      _mulle_objc_metaclass_set_state_bit( meta, MULLE_OBJC_METACLASS_LOAD_SCHEDULED);  // debugging help

      if( mulle_objc_callqueue_add( loads,
                                    (struct _mulle_objc_object *) meta,
                                    MULLE_OBJC_LOAD_METHODID,
                                    imp))
         return( -1);
   }
   return( 0);
}


void
   mulle_objc_methodlist_add_load_to_callqueue_nofail( struct _mulle_objc_methodlist *list,
                                                       struct _mulle_objc_metaclass *meta,
                                                       struct _mulle_objc_callqueue *loads)
{
   if( mulle_objc_methodlist_add_load_to_callqueue( list, meta, loads))
      mulle_objc_universe_fail_errno( _mulle_objc_metaclass_get_universe( meta));
}



// opaque because of struct _mulle_objc_loadcategory being somewat private
mulle_objc_categoryid_t
   _mulle_objc_methodlist_get_categoryid( struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_loadcategory   *category;

   category = list->loadcategory;
   return( category ? category->categoryid : MULLE_OBJC_NO_CATEGORYID);
}


char *
   _mulle_objc_methodlist_get_categoryname( struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_loadcategory   *category;

   category = list->loadcategory;
   return( category ? category->categoryname : NULL);
}


char *
   _mulle_objc_methodlist_get_categoryorigin( struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_loadcategory   *category;

   category = list->loadcategory;
   return( category ? category->origin : NULL);
}
