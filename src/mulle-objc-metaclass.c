//
//  mulle_objc_metaclass.c
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/07
//  Copyright (c) 2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2017 Codeon GmbH.
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
#include "mulle-objc-metaclass.h"

#include "mulle-objc-class.h"
#include "mulle-objc-ivarlist.h"
#include "mulle-objc-propertylist.h"
#include "mulle-objc-universe.h"

#include "include-private.h"


# pragma mark - sanity check

int   mulle_objc_metaclass_is_sane( struct _mulle_objc_metaclass *meta)
{
   if( ! meta)
   {
      errno = EINVAL;
      return( 0);
   }

   return( _mulle_objc_class_is_sane( &meta->base));
}


#pragma mark - walker

mulle_objc_walkcommand_t
   mulle_objc_metaclass_walk( struct _mulle_objc_metaclass   *meta,
                               enum mulle_objc_walkpointertype_t  type,
                               mulle_objc_walkcallback_t   callback,
                               void *parent,
                               void *userinfo)
{
   mulle_objc_walkcommand_t     cmd;

   cmd = mulle_objc_class_walk( _mulle_objc_metaclass_as_class( meta), type, callback, parent, userinfo);
   return( cmd);
}



#pragma mark - class properties

int   mulle_objc_metaclass_add_propertylist( struct _mulle_objc_metaclass *meta,
                                             struct _mulle_objc_propertylist *list)
{
   mulle_objc_propertyid_t                     last;
   struct _mulle_objc_property                 *property;
   struct _mulle_objc_propertylistenumerator   rover;

   if( ! meta || ! list)
   {
      if( ! meta)
      {
         errno = EINVAL;
         return( -1);
      }
      return( 0);  // NULL list is OK — nothing to add
   }

   // validate sort order
   last  = MULLE_OBJC_MIN_UNIQUEID - 1;
   rover = _mulle_objc_propertylist_enumerate( list);
   while( (property = _mulle_objc_propertylistenumerator_next( &rover)))
   {
      if( last > property->propertyid)
      {
         _mulle_objc_propertylistenumerator_done( &rover);
         errno = EDOM;
         return( -1);
      }
      last = property->propertyid;
   }
   _mulle_objc_propertylistenumerator_done( &rover);

   _mulle_concurrent_pointerarray_add( &meta->propertylists, list);
   return( 0);
}


void  mulle_objc_metaclass_add_propertylist_nofail( struct _mulle_objc_metaclass *meta,
                                                    struct _mulle_objc_propertylist *list)
{
   if( mulle_objc_metaclass_add_propertylist( meta, list))
      mulle_objc_universe_fail_errno( _mulle_objc_metaclass_get_universe( meta));
}


struct _mulle_objc_property *
   mulle_objc_metaclass_search_property( struct _mulle_objc_metaclass *meta,
                                         mulle_objc_propertyid_t propertyid)
{
   struct _mulle_objc_propertylist   *list;
   struct _mulle_objc_property       *property;
   unsigned int                       n;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;

   if( ! meta)
      return( NULL);

   n     = mulle_concurrent_pointerarray_get_count( &meta->propertylists);
   rover = mulle_concurrent_pointerarray_reverseenumerate( &meta->propertylists, n);
   while( (list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover)))
   {
      property = _mulle_objc_propertylist_search( list, propertyid);
      if( property)
      {
         mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
         return( property);
      }
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
   return( NULL);
}


#pragma mark - class variables

int   mulle_objc_metaclass_add_ivarlist( struct _mulle_objc_metaclass *meta,
                                         struct _mulle_objc_ivarlist *list)
{
   if( ! meta)
   {
      errno = EINVAL;
      return( -1);
   }
   if( ! list || ! list->n_ivars)
      return( 0);

   _mulle_concurrent_pointerarray_add( &meta->ivarlists, list);
   return( 0);
}


void  mulle_objc_metaclass_add_ivarlist_nofail( struct _mulle_objc_metaclass *meta,
                                                struct _mulle_objc_ivarlist *list)
{
   if( mulle_objc_metaclass_add_ivarlist( meta, list))
      mulle_objc_universe_fail_errno( _mulle_objc_metaclass_get_universe( meta));
}


struct _mulle_objc_ivar *
   mulle_objc_metaclass_search_ivar( struct _mulle_objc_metaclass *meta,
                                     mulle_objc_ivarid_t ivarid)
{
   struct _mulle_objc_ivarlist   *list;
   struct _mulle_objc_ivar       *ivar;
   unsigned int                   n;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;

   if( ! meta)
      return( NULL);

   n     = mulle_concurrent_pointerarray_get_count( &meta->ivarlists);
   rover = mulle_concurrent_pointerarray_reverseenumerate( &meta->ivarlists, n);
   while( (list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover)))
   {
      ivar = _mulle_objc_ivarlist_search( list, ivarid);
      if( ivar)
      {
         mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
         return( ivar);
      }
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
   return( NULL);
}
