//
//  mulle_objc_infraclass.h
//  mulle-objc
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

#include "mulle_objc_infraclass.h"

#include "mulle_objc_class.h"
#include "mulle_objc_classpair.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_ivar.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_property.h"
#include "mulle_objc_propertylist.h"
#include "mulle_objc_universe.h"


void    _mulle_objc_infraclass_plusinit( struct _mulle_objc_infraclass *infra,
                                         struct mulle_allocator *allocator)
{
   _mulle_concurrent_pointerarray_init( &infra->ivarlists, 0, allocator);
   _mulle_concurrent_pointerarray_init( &infra->propertylists, 0, allocator);

   _mulle_concurrent_hashmap_init( &infra->cvars, 0, allocator);
}


void    _mulle_objc_infraclass_plusdone( struct _mulle_objc_infraclass *infra)
{
   _mulle_concurrent_hashmap_done( &infra->cvars);

   _mulle_concurrent_pointerarray_done( &infra->ivarlists);

   // initially room for 2 categories with properties
   _mulle_concurrent_pointerarray_done( &infra->propertylists);
}


# pragma mark - sanitycheck

int   mulle_objc_infraclass_is_sane( struct _mulle_objc_infraclass *infra)
{
   void   *storage;

   if( ! infra)
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! _mulle_objc_class_is_sane( &infra->base))
      return( 0);

   // need at least one possibly empty ivar list
   storage = _mulle_atomic_pointer_nonatomic_read( &infra->ivarlists.storage.pointer);
   if( ! storage || ! mulle_concurrent_pointerarray_get_count( &infra->ivarlists))
   {
      errno = ECHILD;
      return( 0);
   }

   // need at least one possibly empty property list
   storage = _mulle_atomic_pointer_nonatomic_read( &infra->propertylists.storage.pointer);
   if( ! storage || ! mulle_concurrent_pointerarray_get_count( &infra->propertylists))
   {
      errno = ECHILD;
      return( 0);
   }

   return( 1);
}


# pragma mark - properties
//
// doesn't check for duplicates
//
struct _mulle_objc_property   *_mulle_objc_infraclass_search_property( struct _mulle_objc_infraclass *infra,
                                                                       mulle_objc_propertyid_t propertyid)
{
   struct _mulle_objc_property                             *property;
   struct _mulle_objc_propertylist                         *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;

   n     = mulle_concurrent_pointerarray_get_count( &infra->propertylists);
   rover = mulle_concurrent_pointerarray_reverseenumerate( &infra->propertylists, n);

   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      property = _mulle_objc_propertylist_search( list, propertyid);
      if( property)
         return( property);
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);

   if( ! infra->base.superclass)
      return( NULL);
   return( _mulle_objc_infraclass_search_property( (struct _mulle_objc_infraclass *) infra->base.superclass, propertyid));
}


struct _mulle_objc_property  *mulle_objc_infraclass_search_property( struct _mulle_objc_infraclass *infra,
                                                                mulle_objc_propertyid_t propertyid)
{
   assert( propertyid != MULLE_OBJC_NO_PROPERTYID && propertyid != MULLE_OBJC_INVALID_PROPERTYID);

   if( ! infra)
   {
      errno = EINVAL;
      return( NULL);
   }

   return( _mulle_objc_infraclass_search_property( infra, propertyid));
}



int   mulle_objc_infraclass_add_propertylist( struct _mulle_objc_infraclass *infra,
                                              struct _mulle_objc_propertylist *list)
{
   mulle_objc_propertyid_t                     last;
   struct _mulle_objc_property                 *property;
   struct _mulle_objc_propertylistenumerator   rover;
   struct _mulle_objc_universe                  *universe;

   if( ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   if( ! list)
   {
      universe = _mulle_objc_infraclass_get_universe( infra);
      list    = &universe->empty_propertylist;
   }

   /* register instance methods */
   last  = MULLE_OBJC_MIN_UNIQUEID - 1;
   rover = _mulle_objc_propertylist_enumerate( list);
   while( property = _mulle_objc_propertylistenumerator_next( &rover))
   {
      assert( property->propertyid != MULLE_OBJC_NO_PROPERTYID && property->propertyid != MULLE_OBJC_INVALID_PROPERTYID);
      //
      // properties must be sorted by propertyid, so we can binary search them
      // (in the future)
      //
      if( last > property->propertyid)
      {
         errno = EDOM;
         return( -1);
      }
      last = property->propertyid;
   }

   _mulle_objc_propertylistenumerator_done( &rover);

   _mulle_concurrent_pointerarray_add( &infra->propertylists, list);

   return( 0);
}


void   mulle_objc_infraclass_unfailing_add_propertylist( struct _mulle_objc_infraclass *infra,
                                                         struct _mulle_objc_propertylist *list)
{
   if( mulle_objc_infraclass_add_propertylist( infra, list))
   {
      struct _mulle_objc_universe   *universe;

      universe = _mulle_objc_infraclass_get_universe( infra);
      _mulle_objc_universe_raise_fail_errno_exception( universe);
   }
}



# pragma mark - ivar lists

int   mulle_objc_infraclass_add_ivarlist( struct _mulle_objc_infraclass *infra,
                                          struct _mulle_objc_ivarlist *list)
{
   if( ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   if( ! list)
   {
      struct _mulle_objc_universe   *universe;

      universe = _mulle_objc_infraclass_get_universe( infra);
      list    = &universe->empty_ivarlist;
   }

   _mulle_concurrent_pointerarray_add( &infra->ivarlists, list);
   return( 0);
}


void   mulle_objc_infraclass_unfailing_add_ivarlist( struct _mulle_objc_infraclass *infra,
                                                     struct _mulle_objc_ivarlist *list)
{
   if( mulle_objc_infraclass_add_ivarlist( infra, list))
   {
      struct _mulle_objc_universe   *universe;

      universe = _mulle_objc_infraclass_get_universe( infra);
      _mulle_objc_universe_raise_fail_errno_exception( universe);
   }
}

# pragma mark - ivars

//
// doesn't check for duplicates
//
struct _mulle_objc_ivar   *_mulle_objc_infraclass_search_ivar( struct _mulle_objc_infraclass *infra,
                                                               mulle_objc_ivarid_t ivarid)
{
   struct _mulle_objc_ivar                                 *ivar;
   struct _mulle_objc_ivarlist                             *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;

   n     = mulle_concurrent_pointerarray_get_count( &infra->ivarlists);
   rover = mulle_concurrent_pointerarray_reverseenumerate( &infra->ivarlists, n);

   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      ivar = _mulle_objc_ivarlist_search( list, ivarid);
      if( ivar)
         return( ivar);
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);

   if( ! infra->base.superclass)
      return( NULL);
   return( _mulle_objc_infraclass_search_ivar( (struct _mulle_objc_infraclass *) infra->base.superclass, ivarid));
}


struct _mulle_objc_ivar  *mulle_objc_infraclass_search_ivar( struct _mulle_objc_infraclass *infra,
                                                             mulle_objc_ivarid_t ivarid)
{
   assert( ivarid != MULLE_OBJC_NO_IVARID && ivarid != MULLE_OBJC_INVALID_IVARID);

   if( ! infra)
   {
      errno = EINVAL;
      return( NULL);
   }

   return( _mulle_objc_infraclass_search_ivar( infra, ivarid));
}


# pragma mark - ivar walker

typedef   int (*mulle_objc_walk_ivars_callback)( struct _mulle_objc_ivar *, struct _mulle_objc_infraclass *, void *);

int   _mulle_objc_infraclass_walk_ivars( struct _mulle_objc_infraclass *infra,
                                         unsigned int inheritance,
                                         mulle_objc_walk_ivars_callback f,
                                         void *userinfo)
{
   int                                                    rval;
   struct _mulle_objc_ivarlist                            *list;
   struct mulle_concurrent_pointerarrayreverseenumerator  rover;
   unsigned int                                           n;
   struct _mulle_objc_infraclass                          *superclass;
   // todo: need to lock class

   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   n = mulle_concurrent_pointerarray_get_count( &infra->ivarlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &infra->ivarlists, n);

   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( rval = _mulle_objc_ivarlist_walk( list, f, infra, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
         return( _mulle_objc_infraclass_walk_ivars( superclass, inheritance, f, userinfo));
   }

   return( 0);
}

# pragma mark - property walker

typedef   int (*mulle_objc_walk_properties_callback)( struct _mulle_objc_property *, struct _mulle_objc_infraclass *, void *);

int   _mulle_objc_infraclass_walk_properties( struct _mulle_objc_infraclass *infra, unsigned int inheritance , mulle_objc_walk_properties_callback f, void *userinfo)
{
   int                                                     rval;
   struct _mulle_objc_propertylist                         *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;
   struct _mulle_objc_infraclass                           *superclass;

   // todo: need to lock class

   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   n = mulle_concurrent_pointerarray_get_count( &infra->propertylists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &infra->propertylists, n);
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( rval = _mulle_objc_propertylist_walk( list, f, infra, userinfo))
         return( rval);
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
         return( _mulle_objc_infraclass_walk_properties( superclass, inheritance, f, userinfo));
   }

   return( 0);
}


# pragma mark - infraclass walking

static int  print_categoryid( mulle_objc_protocolid_t categoryid,
                              struct _mulle_objc_classpair *pair,
                              void *userinfo)
{
   struct _mulle_objc_universe  *universe;
   
   universe = _mulle_objc_classpair_get_universe( pair);
   fprintf( stderr, "\t%08x \"%s\"\n",
            categoryid,
           _mulle_objc_universe_string_for_categoryid( universe, categoryid));
   return( 0);
}



static char  footer[] = "(so it is not used as protocol class)\n";


struct bouncy_info
{
   void                          *userinfo;
   struct _mulle_objc_universe    *universe;
   void                          *parent;
   mulle_objc_walkcallback_t     callback;
   mulle_objc_walkcommand_t      rval;
};


static int   bouncy_property( struct _mulle_objc_property *property,
                              struct _mulle_objc_infraclass *infra,
                              void *userinfo)
{
   struct bouncy_info   *info;

   info       = userinfo;
   info->rval = (info->callback)( info->universe,
                                  property,
                                  mulle_objc_walkpointer_is_property,
                                  NULL,
                                  infra,
                                  info->userinfo);
   return( mulle_objc_walkcommand_is_stopper( info->rval));
}


static int   bouncy_ivar( struct _mulle_objc_ivar *ivar,
                          struct _mulle_objc_infraclass *infra,
                          void *userinfo)
{
   struct bouncy_info   *info;

   info       = userinfo;
   info->rval = (info->callback)( info->universe,
                                  ivar,
                                  mulle_objc_walkpointer_is_ivar,
                                  NULL,
                                  infra,
                                  info->userinfo);
   return( mulle_objc_walkcommand_is_stopper( info->rval));
}


mulle_objc_walkcommand_t
   mulle_objc_infraclass_walk( struct _mulle_objc_infraclass   *infra,
                               enum mulle_objc_walkpointertype_t  type,
                               mulle_objc_walkcallback_t   callback,
                               void *parent,
                               void *userinfo)
{
   mulle_objc_walkcommand_t     cmd;
   struct bouncy_info           info;

   cmd = mulle_objc_class_walk( _mulle_objc_infraclass_as_class( infra), type, callback, parent, userinfo);
   if( cmd != mulle_objc_walk_ok)
      return( cmd);

   info.callback = callback;
   info.parent   = parent;
   info.userinfo = userinfo;
   info.universe  = _mulle_objc_infraclass_get_universe( infra);

   cmd = _mulle_objc_infraclass_walk_properties( infra, _mulle_objc_infraclass_get_inheritance( infra), bouncy_property, &info);
   if( cmd != mulle_objc_walk_ok)
      return( cmd);

   cmd = _mulle_objc_infraclass_walk_ivars( infra, _mulle_objc_infraclass_get_inheritance( infra), bouncy_ivar, &info);
   return( cmd);
}


# pragma mark - protocolclass check

//
// must be root, must conform to own protocol, must not have ivars
// must not conform to other protocols (it's tempting to conform to NSObject)
// If you conform to NSObject, NSObject methods will override your superclass(!)
//
int    mulle_objc_infraclass_is_protocolclass( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_universe         *universe;
   struct _mulle_objc_classpair       *pair;
   struct _mulle_objc_uniqueidarray   *array;
   int                                is_NSObject;
   int                                has_categories;
   unsigned int                       inheritance;

   if( ! infra)
      return( 0);

   universe = _mulle_objc_infraclass_get_universe( infra);

   if( _mulle_objc_infraclass_get_superclass( infra))
   {
      if( universe->debug.warn.protocolclass)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" "
                             "matches a protocol of same name, but it is "
                             "not a root class %s",
                    universe,
                    _mulle_objc_infraclass_get_name( infra),
                    footer);
      }
      return( 0);
   }

   if( infra->base.allocationsize > sizeof( struct _mulle_objc_objectheader))
   {
      if( universe->debug.warn.protocolclass)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" matches a protocol of the same name"
                 ", but implements instance variables %s",
                   universe,
                    _mulle_objc_infraclass_get_name( infra),
                   footer);
      }
      return( 0);
   }

   pair = _mulle_objc_infraclass_get_classpair( infra);

   if( ! _mulle_objc_classpair_conformsto_protocolid( pair,
                                                     _mulle_objc_infraclass_get_classid( infra)))
   {
      if( universe->debug.warn.protocolclass)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" matches a protocol but does not conform to it %s",
                    universe,
                    _mulle_objc_infraclass_get_name( infra),
                    footer);
      }
      return( 0);
   }

   if( _mulle_objc_classpair_get_protocolclasscount( pair))
   {
      if( universe->debug.warn.protocolclass)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" matches a protocol but also inherits from other protocolclasses %s",
                    universe,
                    _mulle_objc_infraclass_get_name( infra),
                    footer);
      }
      return( 0);
   }

   //
   // check if someone bolted on categories to the protocol. In theory
   // it's OK, but them not being picked up might be a point of
   // confusion (On NSObject though its not worth a warning)
   //
   is_NSObject = _mulle_objc_infraclass_get_classid( infra) ==
                 _mulle_objc_universe_get_rootclassid( universe);
   if( is_NSObject)
      return( 1);

   inheritance = _mulle_objc_class_get_inheritance( _mulle_objc_infraclass_as_class( infra));
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES)
   {
      array = _mulle_atomic_pointer_read( &pair->p_categoryids.pointer);
      has_categories = array->n != 0;
      if( has_categories)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
         {
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" conforms "
                    "to a protocol but has gained some categories, which "
                    "will be ignored.\n",
                    universe,
                    _mulle_objc_infraclass_get_name( infra));

            fprintf( stderr, "Categories:\n");
            _mulle_objc_classpair_walk_categoryids( pair,
                                                   print_categoryid,
                                                   NULL);
         }
      }
   }

   return( 1);
}

