//
//  mulle_objc_propertylist.h
//  mulle-objc-runtime
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
#ifndef mulle_objc_propertylist_h__
#define mulle_objc_propertylist_h__

#include "include.h"

#include "mulle-objc-property.h"
#include "mulle-objc-uniqueid.h"
#include "mulle-objc-walktypes.h"

#include <assert.h>


struct _mulle_objc_infraclass;


struct _mulle_objc_propertylist
{
   unsigned int                 n_properties;
   struct _mulle_objc_property  properties[ 1];
};


static inline unsigned int   _mulle_objc_propertylist_get_count( struct _mulle_objc_propertylist *list)
{
   return( list->n_properties);
}


static inline size_t   mulle_objc_sizeof_propertylist( unsigned int n_properties)
{
   return( sizeof( struct _mulle_objc_propertylist) + (n_properties - 1) * sizeof( struct _mulle_objc_property));
}


MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_walkcommand_t   
  _mulle_objc_propertylist_walk( struct _mulle_objc_propertylist *list,
                                 mulle_objc_walkpropertiescallback_t f,
                                 struct _mulle_objc_infraclass *infra,
                                 void *userinfo);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_property  *
   _mulle_objc_propertylist_find( struct _mulle_objc_propertylist *list,
                                           mulle_objc_propertyid_t propertyid);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_property  *
   _mulle_objc_propertylist_find_for_methodid( struct _mulle_objc_propertylist *list,
                                               mulle_objc_methodid_t methodid);

static inline struct _mulle_objc_property  *_mulle_objc_propertylist_search( struct _mulle_objc_propertylist *list,
                                                                                    mulle_objc_propertyid_t propertyid)
{
   return( _mulle_objc_property_bsearch( list->properties, list->n_properties, propertyid));
}


static inline struct _mulle_objc_property  *_mulle_objc_propertylist_search_smart( struct _mulle_objc_propertylist *list,
                                                                                   mulle_objc_propertyid_t propertyid)

{
   if( list->n_properties >= 14) // 14 is a resarched value for i7
      return( _mulle_objc_propertylist_search( list, propertyid));
   return( _mulle_objc_propertylist_find( list, propertyid));
}


static inline void   mulle_objc_propertylist_sort( struct _mulle_objc_propertylist *list)
{
   if( list)
      mulle_objc_property_sort( list->properties, list->n_properties);
}



# pragma mark - Enumerator

struct _mulle_objc_propertylistenumerator
{
   struct _mulle_objc_property   *sentinel;
   struct _mulle_objc_property   *property;
};


static inline struct  _mulle_objc_propertylistenumerator   
  _mulle_objc_propertylist_enumerate( struct _mulle_objc_propertylist *list)
{
   struct _mulle_objc_propertylistenumerator   rover;

   rover.property   = &list->properties[ 0];
   rover.sentinel = &list->properties[ list->n_properties];

   assert( rover.sentinel >= rover.property);

   return( rover);
}


static inline struct _mulle_objc_property   *_mulle_objc_propertylistenumerator_next( struct _mulle_objc_propertylistenumerator *rover)
{
   return( rover->property < rover->sentinel ? rover->property++ : 0);
}


static inline void  _mulle_objc_propertylistenumerator_done( struct _mulle_objc_propertylistenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


# pragma mark - API

static inline mulle_objc_walkcommand_t   
   mulle_objc_propertylist_walk( struct _mulle_objc_propertylist *list,
                                 mulle_objc_walkpropertiescallback_t f,
                                 struct _mulle_objc_infraclass *infra,
                                 void *userinfo)
{
   if( ! list || ! f)
      return( -1);
   return( _mulle_objc_propertylist_walk( list, f, infra, userinfo));
}


static inline struct  _mulle_objc_propertylistenumerator   
  mulle_objc_propertylist_enumerate( struct _mulle_objc_propertylist *list)
{
   struct _mulle_objc_propertylistenumerator   rover;

   if( ! list)
   {
      rover.property   = NULL;
      rover.sentinel = NULL;
      return( rover);
   }
   return( _mulle_objc_propertylist_enumerate( list));
}


static inline struct _mulle_objc_property   *mulle_objc_propertylistenumerator_next( struct _mulle_objc_propertylistenumerator *rover)
{
   return( rover ? _mulle_objc_propertylistenumerator_next( rover) : NULL);
}


static inline void  mulle_objc_propertylistenumerator_done( struct _mulle_objc_propertylistenumerator *rover)
{
   if( rover)
      _mulle_objc_propertylistenumerator_done( rover);
}



// created by make-container-for.sh _mulle_objc_propertylist

#define mulle_objc_propertylist_for( name, item)                                             \
   assert( sizeof( item) == sizeof( void *));                                                \
   for( struct _mulle_objc_propertylistenumerator                                            \
           rover__ ## item = mulle_objc_propertylist_enumerate( name),                       \
           *rover__  ## item ## __i = (void *) 0;                                            \
        ! rover__  ## item ## __i;                                                           \
        rover__ ## item ## __i = (mulle_objc_propertylistenumerator_done( &rover__ ## item), \
                                   (void *) 1))                                              \
      while( (item = _mulle_objc_propertylistenumerator_next( &rover__ ## item)))


#endif
