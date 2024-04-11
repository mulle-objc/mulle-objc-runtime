//
//  mulle_objc_infraclass.h
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
#include "mulle-objc-infraclass.h"

#include "mulle-objc-class-struct.h"
#include "mulle-objc-class.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-ivar.h"
#include "mulle-objc-ivarlist.h"
#include "mulle-objc-object-convenience.h"
#include "mulle-objc-property.h"
#include "mulle-objc-propertylist.h"
#include "mulle-objc-signature.h"
#include "mulle-objc-universe.h"



/*
 *
 */

void    _mulle_objc_infraclass_plusinit( struct _mulle_objc_infraclass *infra,
                                         struct mulle_allocator *allocator)
{
   struct _mulle_objc_universe   *universe;
   struct mulle_allocator        *objectallocator;

   _mulle_concurrent_pointerarray_init( &infra->ivarlists, 0, allocator);
   _mulle_concurrent_pointerarray_init( &infra->propertylists, 0, allocator);

#if 0
   _mulle_concurrent_hashmap_init( &infra->cvars, 0, allocator);
#endif
#if 0 // zeroed memory, needs no init
   _mulle_concurrent_linkedlist_init( &infra->reuseallocs);
#endif
   universe         = _mulle_objc_infraclass_get_universe( infra);
   objectallocator  = _mulle_objc_universe_get_foundationallocator( universe);
   infra->allocator = objectallocator
                      ? objectallocator
                      : _mulle_objc_universe_get_allocator( universe);
}


void    _mulle_objc_infraclass_plusdone( struct _mulle_objc_infraclass *infra)
{
#if 0
   // this is done earlier now
   _mulle_concurrent_hashmap_done( &infra->cvars);
#endif
   _mulle_objc_infraclass_free_reuseallocs( infra);
   _mulle_concurrent_linkedlist_done( &infra->reuseallocs);
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
   storage = _mulle_atomic_pointer_read_nonatomic( &infra->ivarlists.storage.pointer);
   if( ! storage || ! mulle_concurrent_pointerarray_get_count( &infra->ivarlists))
   {
      errno = ECHILD;
      return( 0);
   }

   // need at least one possibly empty property list
   storage = _mulle_atomic_pointer_read_nonatomic( &infra->propertylists.storage.pointer);
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
      property = _mulle_objc_propertylist_search_smart( list, propertyid);
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
   assert( mulle_objc_uniqueid_is_sane( propertyid));

   if( ! infra)
   {
      errno = EINVAL;
      return( NULL);
   }

   return( _mulle_objc_infraclass_search_property( infra, propertyid));
}


enum
{
   is_getter  = 0x1,
   is_setter  = 0x2,
   is_adder  = 0x4,
   is_remover = 0x8
};


//
// for dynamic properties, create the descriptors
// accessor will contain bits, and methodid already
//
static void
   _mulle_objc_universe_set_setter_name_signature( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_descriptor  *accessor,
                                                   char *prefix,
                                                   char *name,
                                                   char *signature,
                                                   size_t s_len)
{
   size_t         n_len;
   size_t         len;
   size_t         p_len;
   unsigned int   size;

   assert( accessor->bits);
   assert( accessor->methodid);

   p_len = strlen( prefix);
   n_len = strlen( name);
   len   = n_len > s_len ? n_len : s_len;
   {
      // windows compiler can't do this
      // char   buf[ len + 60 + p_len + 1];
      char  buf[ 512];

      if( (len + 60 + p_len + 1) > sizeof( buf))
         abort();

      sprintf( buf, "%s%s:", prefix, name);
      if( buf[ p_len] >= 'a' && buf[ p_len] <= 'z')
         buf[ p_len] += 'A' - 'a';
      accessor->name = _mulle_objc_universe_strdup( universe, buf);

      assert( mulle_objc_methodid_from_string( accessor->name) == accessor->methodid);

      mulle_objc_signature_supply_size_and_alignment( signature, &size, NULL);
      sprintf( buf, "v%d@0:%d%.*s%d",
                                  (int) (size + sizeof( void *) + sizeof( void *)),
                                  (int) sizeof( void *),
                                  (int) s_len, signature,
                                  (int) (sizeof( void *) + sizeof( void *)));
      accessor->signature = _mulle_objc_universe_strdup( universe, buf);
   }
}


static inline int   count_bits( unsigned int bits)
{
   int   n;

   n = 0;
   while( bits)
   {
      ++n;
      bits >>= 1;
   }
   return( n);
}


static void
   _mulle_objc_universe_register_descriptors_for_property( struct _mulle_objc_universe *universe,
                                                           struct _mulle_objc_property *property,
                                                           unsigned int bits)
{
   struct _mulle_objc_descriptor  *space;
   size_t                         s_len;
   char                           *s_type;
   char                           *e_type;

   assert( bits && bits < 0x10);

   // get @encode from signature
   s_type = property->signature;
   e_type = strchr( s_type, ',');
   e_type = e_type ? e_type : &s_type[ strlen( s_type)];
   s_len  = e_type - s_type;

   // allocate as many descriptors as needed

   space = _mulle_objc_universe_calloc( universe,
                                        count_bits( bits),
                                        sizeof( struct _mulle_objc_descriptor));

   MULLE_C_ASSERT( sizeof( int) <= 8);  // %d -> 20 signs max for 64 bit

   if( bits & is_getter)  // getter
   {
      struct _mulle_objc_descriptor  *getter;
      // windows compiler can't do this
      // char   buf[ s_len + 40 + 3 + 1];
      char  buf[ 512];

      if( (s_len + 40 + 3 + 1) > sizeof( buf))
         abort();

      getter            = space++;
      getter->bits      = (unsigned long) _mulle_objc_methodfamily_getter << _mulle_objc_methodfamily_shift;
      getter->methodid  = property->propertyid;
      getter->name      = property->name;

      sprintf( buf, "%.*s%d@0:%d", (int) s_len, s_type,
                                   (int) (sizeof( void *) + sizeof( void *)),
                                   (int) sizeof( void *));
      getter->signature = _mulle_objc_universe_strdup( universe, buf);

      _mulle_objc_universe_register_descriptor_nofail( universe, getter);
   }


   if( bits & is_setter)
   {
      struct _mulle_objc_descriptor  *setter;

      setter            = space++;
      setter->bits      = (unsigned long) _mulle_objc_methodfamily_setter << _mulle_objc_methodfamily_shift;
      setter->methodid  = property->setter;

      _mulle_objc_universe_set_setter_name_signature( universe,
                                                      setter,
                                                      "set",
                                                      property->name,
                                                      s_type,
                                                      s_len);
      _mulle_objc_universe_register_descriptor_nofail( universe, setter);
   }

   if( bits & is_adder)
   {
      struct _mulle_objc_descriptor  *adder;

      adder            = space++;
      adder->bits      = (unsigned long) _mulle_objc_methodfamily_adder << _mulle_objc_methodfamily_shift;
      adder->methodid  = property->adder;

      _mulle_objc_universe_set_setter_name_signature( universe,
                                                      adder,
                                                      "addTo",
                                                      property->name,
                                                      s_type,
                                                      s_len);
      _mulle_objc_universe_register_descriptor_nofail( universe, adder);
   }

   if( bits & is_remover)
   {
      struct _mulle_objc_descriptor  *remover;

      remover            = space++;
      remover->bits      = (unsigned long) _mulle_objc_methodfamily_remover << _mulle_objc_methodfamily_shift;
      remover->methodid  = property->remover;

      _mulle_objc_universe_set_setter_name_signature( universe,
                                                      remover,
                                                      "removeFrom",
                                                      property->name,
                                                      s_type,
                                                      s_len);
      _mulle_objc_universe_register_descriptor_nofail( universe, remover);
   }
}


static int   _mulle_objc_infraclass_add_propertylist( struct _mulle_objc_infraclass *infra,
                                                      struct _mulle_objc_propertylist *list)
{
   mulle_objc_propertyid_t                     last;
   struct _mulle_objc_property                 *property;
   struct _mulle_objc_propertylistenumerator   rover;
   struct _mulle_objc_universe                 *universe;
   int                                         bit_isset;

   universe = _mulle_objc_infraclass_get_universe( infra);

   /* register instance methods */
   last      = MULLE_OBJC_MIN_UNIQUEID - 1;
   rover     = _mulle_objc_propertylist_enumerate( list);
   bit_isset = 0;
   while( property = _mulle_objc_propertylistenumerator_next( &rover))
   {
      assert( mulle_objc_uniqueid_is_sane( property->propertyid));

      //
      // properties must be sorted by propertyid, so we can binary search them
      //
      if( last > property->propertyid)
      {
         errno = EDOM;
         return( -1);
      }
      last = property->propertyid;

      // it seems clearing readonly is incompatible, though it might be
      // backed by an ivar so don't do it
      if( ! bit_isset)
      {
         if( ! (property->bits & _mulle_objc_property_readonly) &&
               (property->bits & (_mulle_objc_property_setterclear|_mulle_objc_property_autoreleaseclear)))
         {
            _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_HAS_CLEARABLE_PROPERTY);
            bit_isset = 1;
         }
      }

      //
      // if we are a dynamic property, ensure that setter/getter have been
      // defined, otherwise create them now (for MulleGenericObject forward:
      // to work)
      //
      if( property->bits & _mulle_objc_property_dynamic)
      {
         int   bits;

         bits = 0;
         if( ! _mulle_objc_universe_lookup_descriptor( universe, property->getter))
            bits = is_getter;
         if( property->setter)
            if( ! _mulle_objc_universe_lookup_descriptor( universe, property->setter))
               bits |= is_setter;
         if( property->adder)
            if( ! _mulle_objc_universe_lookup_descriptor( universe, property->adder))
               bits |= is_adder;
         if( property->remover)
            if( ! _mulle_objc_universe_lookup_descriptor( universe, property->remover))
               bits |= is_remover;
         if( bits)
            _mulle_objc_universe_register_descriptors_for_property( universe, property, bits);
      }
   }
   _mulle_objc_propertylistenumerator_done( &rover);

   _mulle_concurrent_pointerarray_add( &infra->propertylists, list);

   return( 0);
}


int   mulle_objc_infraclass_add_propertylist( struct _mulle_objc_infraclass *infra,
                                              struct _mulle_objc_propertylist *list)
{
   struct _mulle_objc_universe   *universe;

   if( ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   if( ! list)
   {
      if( _mulle_concurrent_pointerarray_get_count( &infra->propertylists) != 0)
         return( 0);

      universe = _mulle_objc_infraclass_get_universe( infra);
      list     = &universe->empty_propertylist;
      _mulle_concurrent_pointerarray_add( &infra->propertylists, list);
      return( 0);
   }

   return( _mulle_objc_infraclass_add_propertylist( infra, list));
}


void   mulle_objc_infraclass_add_propertylist_nofail( struct _mulle_objc_infraclass *infra,
                                                      struct _mulle_objc_propertylist *list)
{
   struct _mulle_objc_universe   *universe;

   if( mulle_objc_infraclass_add_propertylist( infra, list))
   {
      universe = _mulle_objc_infraclass_get_universe( infra);
      mulle_objc_universe_fail_errno( universe);
   }
}


# pragma mark - ivar lists

int   mulle_objc_infraclass_add_ivarlist( struct _mulle_objc_infraclass *infra,
                                          struct _mulle_objc_ivarlist *list)
{
   struct _mulle_objc_universe   *universe;

   if( ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   // only add empty list, if there is nothing there yet
   if( ! list)
   {
      if( _mulle_concurrent_pointerarray_get_count( &infra->ivarlists) != 0)
         return( 0);

      universe = _mulle_objc_infraclass_get_universe( infra);
      list     = &universe->empty_ivarlist;
   }

   _mulle_concurrent_pointerarray_add( &infra->ivarlists, list);
   return( 0);
}


void   mulle_objc_infraclass_add_ivarlist_nofail( struct _mulle_objc_infraclass *infra,
                                                     struct _mulle_objc_ivarlist *list)
{
   struct _mulle_objc_universe   *universe;

   if( mulle_objc_infraclass_add_ivarlist( infra, list))
   {
      universe = _mulle_objc_infraclass_get_universe( infra);
      mulle_objc_universe_fail_errno( universe);
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
      ivar = _mulle_objc_ivarlist_search_smart( list, ivarid);
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
   assert( mulle_objc_uniqueid_is_sane( ivarid));

   if( ! infra)
   {
      errno = EINVAL;
      return( NULL);
   }

   return( _mulle_objc_infraclass_search_ivar( infra, ivarid));
}


# pragma mark - ivar walker

mulle_objc_walkcommand_t
	_mulle_objc_infraclass_walk_ivars( struct _mulle_objc_infraclass *infra,
                                      unsigned int inheritance,
                                      mulle_objc_walkivarscallback_t f,
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
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &infra->ivarlists, n);

   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( rval = _mulle_objc_ivarlist_walk( list, f, infra, userinfo))
      {
         if( rval < mulle_objc_walk_ok)
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

   return( mulle_objc_walk_ok);
}

# pragma mark - property walker

mulle_objc_walkcommand_t
	_mulle_objc_infraclass_walk_properties( struct _mulle_objc_infraclass *infra,
                                           unsigned int inheritance,
                                           mulle_objc_walkpropertiescallback_t f,
                                           void *userinfo)
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

loop:
   n = mulle_concurrent_pointerarray_get_count( &infra->propertylists);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &infra->propertylists, n);
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( (rval = _mulle_objc_propertylist_walk( list, f, infra, userinfo)))
         return( rval);
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
      {
         infra = superclass;
         goto loop;  // avoid recursion stack in this simple case
      }
   }

   return( mulle_objc_walk_ok);
}


# pragma mark - infraclass walking

static mulle_objc_walkcommand_t  
   print_categoryid( mulle_objc_protocolid_t categoryid,
                     struct _mulle_objc_classpair *pair,
                     void *userinfo)
{
   struct _mulle_objc_universe  *universe;

   universe = _mulle_objc_classpair_get_universe( pair);
   fprintf( stderr, "\t%08lx \"%s\"\n",
            (unsigned long) categoryid,
           _mulle_objc_universe_describe_categoryid( universe, categoryid));
   return( 0);
}



static char  footer[] = "(so it is not functioning as a protocol class)\n";


struct bouncy_info
{
   void                          *userinfo;
   struct _mulle_objc_universe   *universe;
   void                          *parent;
   mulle_objc_walkcallback_t     callback;
   mulle_objc_walkcommand_t      rval;
};


static mulle_objc_walkcommand_t   
   bouncy_property( struct _mulle_objc_property *property,
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


static mulle_objc_walkcommand_t   
   bouncy_ivar( struct _mulle_objc_ivar *ivar,
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
                               mulle_objc_walkcallback_t callback,
                               void *parent,
                               void *userinfo)
{
   mulle_objc_walkcommand_t     cmd;
   struct bouncy_info           info;
   unsigned int                 inheritance;

   cmd = mulle_objc_class_walk( _mulle_objc_infraclass_as_class( infra),
                                 type,
                                 callback,
                                 parent,
                                 userinfo);
   if( cmd != mulle_objc_walk_ok)
      return( cmd);

   info.callback = callback;
   info.parent   = parent;
   info.userinfo = userinfo;
   info.universe = _mulle_objc_infraclass_get_universe( infra);
   inheritance   = _mulle_objc_infraclass_get_inheritance( infra);
   cmd = _mulle_objc_infraclass_walk_properties( infra, inheritance, bouncy_property, &info);
   if( cmd != mulle_objc_walk_ok)
      return( cmd);

   cmd = _mulle_objc_infraclass_walk_ivars( infra, inheritance, bouncy_ivar, &info);
   return( cmd);
}


# pragma mark - protocolclass check

//
// Checks that infra must be root, must conform to own protocol, must not have
// ivars must not conform to other protocols (it's tempting to conform to NSObject)
// If you conform to NSObject, NSObject methods will override your superclass(!)
//
static int   _mulle_objc_infraclass_is_protocolclass( struct _mulle_objc_infraclass *infra,
                                                      int warn)
{
   struct _mulle_objc_universe         *universe;
   struct _mulle_objc_classpair       *pair;
   struct _mulle_objc_uniqueidarray   *array;
   int                                is_NSObject;
   int                                has_categories;
   unsigned int                       inheritance;

   universe = _mulle_objc_infraclass_get_universe( infra);

   if( _mulle_objc_infraclass_get_superclass( infra))
   {
      if( warn)
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
      if( warn)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" "
                             "matches a protocol of the same name, but "
                             "implements instance variables %s",
                        universe,
                        _mulle_objc_infraclass_get_name( infra),
                        footer);
      }           return( 0);
   }

   pair = _mulle_objc_infraclass_get_classpair( infra);
   if( ! _mulle_objc_classpair_conformsto_protocolid( pair,
                                                     _mulle_objc_infraclass_get_classid( infra)))
   {
      if( warn)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" "
                             "matches a protocol but does not conform to it %s",
                        universe,
                        _mulle_objc_infraclass_get_name( infra),
                        footer);
      }
      return( 0);
   }

   if( _mulle_objc_classpair_get_protocolclasscount( pair))
   {
      if( warn)
      {
         if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_universe %p warning: class \"%s\" "
                             "matches a protocol but also inherits from other "
                             "protocolclasses %s",
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
         if( warn)
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
                                                       MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS,
                                                       print_categoryid,
                                                       NULL);
            }
         }
      }
   }

   return( 1);
}

// see _mulle_objc_infraclass_is_protocolclass, this
int   mulle_objc_infraclass_is_protocolclass( struct _mulle_objc_infraclass *infra)
{
   if( ! infra)
      return( 0);
   return( _mulle_objc_infraclass_is_protocolclass( infra, 0));
}


int   mulle_objc_infraclass_check_protocolclass( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_universe   *universe;

   if( ! infra)
      return( 0);

   universe = _mulle_objc_infraclass_get_universe( infra);
   return( _mulle_objc_infraclass_is_protocolclass( infra, universe->debug.warn.protocolclass));
}


// unused...
// static struct _mulle_objc_method  *
//    _mulle_objc_infraclass_search_method_noinherit( struct _mulle_objc_infraclass *infra,
//                                                    mulle_objc_methodid_t methodid)
// {
//    struct _mulle_objc_searcharguments   search;
//    struct _mulle_objc_method            *method;
//    struct _mulle_objc_metaclass         *meta;

//    _mulle_objc_searcharguments_init_default( &search, methodid);
//    meta   = _mulle_objc_infraclass_get_metaclass( infra);
//    method = mulle_objc_class_search_method( _mulle_objc_metaclass_as_class( meta),
//                                             &search,
//                                             ~MULLE_OBJC_CLASS_DONT_INHERIT_CLASS,  // inherit nothing
//                                             NULL);
//    return( method);
// }


static void   _mulle_objc_infraclass_call_unloadmethod( struct _mulle_objc_infraclass *infra,
                                                        struct _mulle_objc_method *method,
                                                        char *name,
                                                        char *categoryname)
{
   struct _mulle_objc_universe     *universe;
   mulle_objc_implementation_t     imp;

   universe = _mulle_objc_infraclass_get_universe( infra);
   if( universe->debug.trace.initialize)
   {
      mulle_objc_universe_trace( universe,
                                 "call +%s on class #%ld %s%s%s%s",
                                 name,
                                 _mulle_objc_classpair_get_classindex( _mulle_objc_infraclass_get_classpair( infra)),
                                 _mulle_objc_infraclass_get_name( infra),
                                 categoryname ? "( " : "",
                                 categoryname ? categoryname : "",
                                 categoryname ? ")" : "");
   }

   imp = _mulle_objc_method_get_implementation( method);
   mulle_objc_implementation_invoke( imp,
                                     infra,
                                     _mulle_objc_method_get_methodid( method),
                                     infra);
}




static struct _mulle_objc_method  *
   _mulle_objc_infraclass_search_method_noinherit_noinitialize( struct _mulle_objc_infraclass *infra,
                                                                mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_method            *method;
   struct _mulle_objc_metaclass         *meta;

   if( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
      return( NULL);

   _mulle_objc_searcharguments_init_default( &search, methodid);
   meta   = _mulle_objc_infraclass_get_metaclass( infra);
   method = mulle_objc_class_search_method( _mulle_objc_metaclass_as_class( meta),
                                            &search,
                                            ~MULLE_OBJC_CLASS_DONT_INHERIT_CLASS,  // inherit nothing
                                            NULL);
   return( method);
}


//
// these will only be called if +initialize has run
//
void   _mulle_objc_infraclass_call_willfinalize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_method     *method;

   method = _mulle_objc_infraclass_search_method_noinherit_noinitialize( infra,
                                                                         MULLE_OBJC_WILLFINALIZE_METHODID);
   if( method)
      _mulle_objc_infraclass_call_unloadmethod( infra, method, "willFinalize", NULL);
}


void   _mulle_objc_infraclass_call_finalize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_method     *method;

   method = _mulle_objc_infraclass_search_method_noinherit_noinitialize( infra,
                                                                         MULLE_OBJC_FINALIZE_METHODID);
   if( method)
   {
      _mulle_objc_infraclass_call_unloadmethod( infra, method, "finalize", NULL);
      _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_FINALIZE_DONE);
   }
}


// in reverse order
void   _mulle_objc_infraclass_call_unload( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass         *meta;
   struct _mulle_objc_class             *cls;
   struct _mulle_objc_method            *method;
   int                                  inheritance;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;

   meta = _mulle_objc_infraclass_get_metaclass( infra);
   cls  = _mulle_objc_metaclass_as_class( meta);

   inheritance = MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS|MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS;

   // search will find last recently added category first
   // but we need to call all
   _mulle_objc_searcharguments_init_default( &search, MULLE_OBJC_UNLOAD_METHODID);
   for(;;)
   {
      method = mulle_objc_class_search_method( cls, &search, inheritance, &result);
      if( ! method)
         break;

      _mulle_objc_infraclass_call_unloadmethod( infra,
                                                method,
                                                "unload",
                                                _mulle_objc_methodlist_get_categoryname( result.list));
      _mulle_objc_searcharguments_init_previous( &search, method);
   }
}

