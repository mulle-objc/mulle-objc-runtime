//
//  mulle_objc_classpair.c
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/08.
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
#include "mulle-objc-classpair.h"

#include "include-private.h"

#include "mulle-objc-class-struct.h"
#include "mulle-objc-class.h"
#include "mulle-objc-retain-release.h"
#include "mulle-objc-universe-class.h"
#include "mulle-objc-universe.h"


void    _mulle_objc_classpair_plusinit( struct _mulle_objc_classpair *pair,
                                        struct mulle_allocator *allocator)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_classpair_get_universe( pair);
   assert( universe);

   if( mulle_thread_mutex_init( &pair->lock))
      abort();

   _mulle_concurrent_pointerarray_init( &pair->protocolclasses, 0, allocator);

   _mulle_atomic_pointer_nonatomic_write( &pair->p_protocolids.pointer,
                                          &universe->empty_uniqueidarray);
   _mulle_atomic_pointer_nonatomic_write( &pair->p_categoryids.pointer,
                                          &universe->empty_uniqueidarray);

//   pair->taoprotection = _MULLE_OBJC_CLASSPAIR_TAO_MAGIC;
}


void    _mulle_objc_classpair_plusdone( struct _mulle_objc_classpair *pair,
                                        struct mulle_allocator *allocator)
{
   struct _mulle_objc_uniqueidarray   *array;
   struct _mulle_objc_universe        *universe;

   universe = _mulle_objc_classpair_get_universe( pair);
   assert( universe);

   mulle_thread_mutex_done( &pair->lock);

   array = _mulle_atomic_pointer_nonatomic_read( &pair->p_protocolids.pointer);
   if( array != &universe->empty_uniqueidarray)
      mulle_objc_uniqueidarray_abafree( array, allocator);

   array = _mulle_atomic_pointer_nonatomic_read( &pair->p_categoryids.pointer);
   if( array != &universe->empty_uniqueidarray)
      mulle_objc_uniqueidarray_abafree( array, allocator);

   _mulle_concurrent_pointerarray_done( &pair->protocolclasses);
}



//
// this is low level, you don't use it
// the outcome is a class (piece) that in itself must be sane
//


// (no instances here yet..)
//   @interface NSObject
//     NSObject ---isa--> meta-NSObject  (1)
//     NSObject ---superclass--> nil
//     meta-NSObject ---isa--> meta-NSObject (2)
//     meta-NSObject ---superclass--> NSObject (3)
//
//   @interface Foo : NSObject
//     Foo ---isa--> meta-Foo (1)
//     Foo ---superclass--> NSObject
//     meta-Foo ---isa--> meta-NSObject (2)
//     meta-Foo ---superclass--> meta-NSObject
//
//   @interface Bar : Foo
//     Bar ---isa--> meta-Bar (1)
//     Bar ---superclass--> Foo
//     meta-Bar ---isa--> meta-NSObject (2)
//     meta-Bar ---superclass--> meta-Foo
//
// a class-pair is a class and a meta-class
//
// every class has a superclass except the root class
// every meta-class has a superclass
//
// (1) every class's isa points to its meta-class sibling
// (2) every meta-class's isa points to the root meta-class
// (3) the root meta-class's superclass is the root class
//
// what is the use of the meta-class. It's basically just
// there to hold a method cache for class methods.
//
struct _mulle_objc_classpair *
   mulle_objc_universe_new_classpair( struct _mulle_objc_universe *universe,
                                      mulle_objc_classid_t  classid,
                                      char *name,
                                      size_t instancesize,
                                      size_t classextra,
                                      struct _mulle_objc_infraclass *superclass)
{
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_metaclass   *super_meta;
   struct _mulle_objc_metaclass   *super_meta_isa;
   struct mulle_allocator         *allocator;
   mulle_objc_classid_t           correct;
   size_t                         size;

   if( ! universe)
   {
      errno = EINVAL;
      return( NULL);
   }

   allocator = _mulle_objc_universe_get_allocator( universe);
   if( _mulle_objc_universe_is_uninitialized( universe) || ! allocator)
   {
      fprintf( stderr, "mulle_objc_universe error: The universe %p has not "
                       "been set up yet. You probably forgot to link the "
                       "startup library\n", universe);
      errno = ENXIO;
      return( NULL);
   }

   if( classid == MULLE_OBJC_NO_CLASSID || classid == MULLE_OBJC_INVALID_CLASSID)
   {
      errno = EINVAL;
      return( NULL);
   }

   if( ! name || ! strlen( name))
   {
      errno = EINVAL;
      return( NULL);
   }

   correct = mulle_objc_classid_from_string( name);
   if( classid != correct)
   {
      fprintf( stderr, "mulle_objc_universe error: Class \"%s\" should have "
                       "classid %08lx but has classid %08lx\n", name,
                       (unsigned long) correct, (unsigned long) classid);
      errno = EINVAL;
      return( NULL);
   }

   super_meta     = NULL;
   super_meta_isa = NULL;
   if( superclass)
   {
      super_meta     = _mulle_objc_infraclass_get_metaclass( superclass);
      super_meta_isa = _mulle_objc_class_get_metaclass( &super_meta->base);
      assert( super_meta_isa);
   }

   // classes are freed by hand so don't use the gifting calloc
   size = mulle_objc_classpair_size( classextra);
   pair = _mulle_allocator_calloc( allocator, 1, size);

   // classes have no extra meta, should they though ?
   _mulle_objc_objectheader_init( &pair->infraclassheader, &pair->metaclass.base, 0, _mulle_objc_memory_is_zeroed);
   _mulle_objc_objectheader_init( &pair->metaclassheader,
                                  super_meta_isa ? _mulle_objc_metaclass_as_class( super_meta_isa)
                                                 : _mulle_objc_infraclass_as_class( &pair->infraclass),
                                  0,
                                  _mulle_objc_memory_is_zeroed);

   // _mulle_objc_objectheader_init will set _thread, because we haven't
   // set the proper bits yet...
#if MULLE_OBJC_TAO_OBJECT_HEADER
   pair->infraclassheader._thread = 0;
   pair->metaclassheader._thread  = 0;
#endif

   _mulle_objc_object_constantify_noatomic( &pair->infraclass.base);
   _mulle_objc_object_constantify_noatomic( &pair->metaclass.base);

   _mulle_objc_class_init( &pair->infraclass.base,
                           name,
                           instancesize,
                           universe->foundation.headerextrasize,
                           classid,
                           superclass ? &superclass->base : NULL,
                           universe);
   _mulle_objc_class_init( &pair->metaclass.base,
                           name,
                           sizeof( struct _mulle_objc_class),
                           universe->foundation.headerextrasize,
                           classid,
                           super_meta ? &super_meta->base : &pair->infraclass.base,
                           universe);

   _mulle_objc_infraclass_plusinit( &pair->infraclass, allocator);
   _mulle_objc_metaclass_plusinit( &pair->metaclass, allocator);
   _mulle_objc_classpair_plusinit( pair, allocator);

   _mulle_objc_class_set_infraclass( &pair->metaclass.base, &pair->infraclass);

   return( pair);
}


void   _mulle_objc_classpair_free( struct _mulle_objc_classpair *pair,
                                   struct mulle_allocator *allocator)
{
   assert( pair);
   assert( allocator);

   _mulle_objc_infraclass_plusdone( &pair->infraclass);
   _mulle_objc_class_done( _mulle_objc_infraclass_as_class( &pair->infraclass),
                                                            allocator);
   _mulle_objc_metaclass_plusdone( &pair->metaclass);
   _mulle_objc_class_done( _mulle_objc_metaclass_as_class( &pair->metaclass),
                                                           allocator);
   _mulle_objc_classpair_plusdone( pair, allocator);

   _mulle_allocator_free( allocator, pair);
}


void   mulle_objc_classpair_free( struct _mulle_objc_classpair *pair,
                                  struct mulle_allocator *allocator)
{
   struct _mulle_objc_universe   *universe;

   if( ! pair)
      return;

   if( ! allocator)
   {
      universe = _mulle_objc_classpair_get_universe( pair);
      errno    = EINVAL;
      mulle_objc_universe_fail_errno( universe);
   }

   _mulle_objc_classpair_free( pair, allocator);
}


mulle_objc_walkcommand_t
   mulle_objc_classpair_walk( struct _mulle_objc_classpair *pair,
                              mulle_objc_walkcallback_t callback,
                              void *userinfo)
{
   mulle_objc_walkcommand_t        cmd;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_universe     *universe;

   universe = _mulle_objc_classpair_get_universe( pair);

   cmd = (*callback)( universe, pair, mulle_objc_walkpointer_is_classpair, NULL, NULL, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   infra = _mulle_objc_classpair_get_infraclass( pair);
   cmd   = mulle_objc_infraclass_walk( infra, mulle_objc_walkpointer_is_infraclass,
                                       callback, universe, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   meta = _mulle_objc_classpair_get_metaclass( pair);
   cmd  = mulle_objc_metaclass_walk( meta, mulle_objc_walkpointer_is_metaclass,
                                     callback, infra, userinfo);
   return( cmd);
}


#pragma mark - protocolids, categories common

void  _mulle_objc_classpair_set_uniqueidarray( struct _mulle_objc_classpair *pair,
                                               mulle_atomic_pointer_t *pointer,
                                               struct _mulle_objc_uniqueidarray *array)
{
   void                          *old;
   struct mulle_allocator        *allocator;
   struct _mulle_objc_universe   *universe;

   do
      old = _mulle_atomic_pointer_read( pointer);
   while( ! _mulle_atomic_pointer_weakcas( pointer, array, old));

   universe = _mulle_objc_classpair_get_universe( pair);
   if( array == &universe->empty_uniqueidarray)
      return;

   allocator = _mulle_objc_universe_get_allocator( universe);
   mulle_objc_uniqueidarray_abafree( array, allocator);
}


void  _mulle_objc_classpair_add_uniqueidarray_ids( struct _mulle_objc_classpair *pair,
                                                   mulle_atomic_pointer_t *pointer,
                                                   unsigned int n,
                                                   mulle_objc_uniqueid_t *uniqueids)
{
   struct _mulle_objc_uniqueidarray   *array;
   struct _mulle_objc_uniqueidarray   *copy;
   struct mulle_allocator             *allocator;
   struct _mulle_objc_universe        *universe;
   unsigned int                       i;

   universe   = _mulle_objc_classpair_get_universe( pair);
   allocator = _mulle_objc_universe_get_allocator( universe);

   copy = NULL;
   // emit what we have collected
   do
   {
      if( copy)
         mulle_objc_uniqueidarray_free( copy, allocator);

      array = _mulle_atomic_pointer_read( pointer);
      copy  = _mulle_objc_uniqueidarray_by_adding_ids( array, n, uniqueids, allocator);
   }
   while( ! _mulle_atomic_pointer_weakcas( pointer, copy, array));

   if( array != &universe->empty_uniqueidarray)
      mulle_objc_uniqueidarray_abafree( array, allocator);

   if( universe->debug.trace.protocol_add)
      for( i = 0; i < n; i++)
         mulle_objc_universe_trace( universe,
                                    "bulk add protocol %08x \"%s\" to class %08x \"%s\"",
                                    uniqueids[ i],
                                    _mulle_objc_universe_describe_categoryid( universe, uniqueids[ i]),
                                    _mulle_objc_classpair_get_classid( pair),
                                    _mulle_objc_classpair_get_name( pair));
}


#pragma mark - categories


void   _mulle_objc_classpair_add_categoryid( struct _mulle_objc_classpair *pair,
                                             mulle_objc_categoryid_t categoryid)
{
   assert( pair);
   assert( mulle_objc_uniqueid_is_sane( categoryid));
   assert( ! _mulle_objc_classpair_has_categoryid( pair, categoryid));

   _mulle_objc_classpair_add_categoryids( pair, 1, &categoryid);
}


//MULLE_C_NEVER_INLINE
//static void   _mulle_objc_universe_fail_einval( struct _mulle_objc_universe *universe)
//{
//  mulle_objc_universe_fail_code( universe, EINVAL);
//}


MULLE_C_NEVER_INLINE
static void   _mulle_objc_classpair_fail_einval( struct _mulle_objc_classpair *pair)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_classpair_get_universe( pair);
   mulle_objc_universe_fail_code( universe, EINVAL);
}


mulle_objc_walkcommand_t
	_mulle_objc_classpair_walk_categoryids( struct _mulle_objc_classpair *pair,
                                           unsigned int inheritance,
                                           mulle_objc_walkcategoryidscallback_t f,
                                           void *userinfo)
{
   mulle_objc_walkcommand_t           rval;
   mulle_objc_categoryid_t            *p;
   mulle_objc_categoryid_t            *sentinel;
   struct _mulle_objc_uniqueidarray   *array;
   struct _mulle_objc_infraclass      *infra;
   struct _mulle_objc_infraclass      *superclass;
   struct _mulle_objc_classpair       *superpair;

   array    = _mulle_atomic_pointer_read( &pair->p_categoryids.pointer);
   p        = array->entries;
   sentinel = &p[ array->n];
   while( p < sentinel)
   {
      if( rval = (*f)( *p++, pair, userinfo))
      {
         if( rval < mulle_objc_walk_ok)
            errno = ENOENT;
         return( rval);
      }
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      infra      = _mulle_objc_classpair_get_infraclass( pair);
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
      {
         superpair = _mulle_objc_infraclass_get_classpair( superclass);
         return( _mulle_objc_classpair_walk_categoryids( superpair, inheritance, f, userinfo));
      }
   }

   return( 0);
}


void   mulle_objc_classpair_add_categoryid_nofail( struct _mulle_objc_classpair *pair,
                                                   mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_infraclass  *infra;
   struct _mulle_objc_universe    *universe;

   if( ! pair)
      _mulle_objc_classpair_fail_einval( pair);
   if( ! mulle_objc_uniqueid_is_sane( categoryid))
      _mulle_objc_classpair_fail_einval( pair);

   universe =_mulle_objc_classpair_get_universe( pair);
   if( ! universe)
      _mulle_objc_classpair_fail_einval( pair);

   // adding a category twice is very bad
   if( _mulle_objc_classpair_has_categoryid( pair, categoryid))
   {
      fprintf( stderr, "mulle_objc_universe %p error: category %08lx for"
                       " class %08lx \"%s\" is already loaded\n",
              universe,
              (unsigned long) categoryid,
              (unsigned long) _mulle_objc_classpair_get_classid( pair),
              _mulle_objc_classpair_get_name( pair));
      _mulle_objc_classpair_fail_einval( pair);
   }


   infra = _mulle_objc_classpair_get_infraclass( pair);
   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_IS_PROTOCOLCLASS))
   {
      if( universe->debug.warn.protocolclass)
         if( universe->foundation.rootclassid != _mulle_objc_classpair_get_classid( pair))
            fprintf( stderr, "mulle_objc_universe %p warning: class %08lx \"%s\""
                             " is a protocolclass and gains a"
                             " category %08lx \"%s( %s)\"\n",
                    universe,
                    (unsigned long) _mulle_objc_classpair_get_classid( pair),
                    _mulle_objc_classpair_get_name( pair),
                    (unsigned long)  categoryid,
                    _mulle_objc_classpair_get_name( pair),
                    _mulle_objc_universe_describe_categoryid( universe, categoryid));
   }

   _mulle_objc_classpair_add_categoryid( pair, categoryid);

   if( universe->debug.trace.category_add || universe->debug.trace.dependency)
      mulle_objc_universe_trace( universe,
                                 "added category %08lx \"%s\" to class %08lx \"%s\"",
                                 (unsigned long)  categoryid,
                                 _mulle_objc_universe_describe_categoryid( universe, categoryid),
                                 (unsigned long)  _mulle_objc_classpair_get_classid( pair),
                                 _mulle_objc_classpair_get_name( pair));

}


#pragma mark - protocolclasses

void   _mulle_objc_classpair_add_protocolclass( struct _mulle_objc_classpair *pair,
                                                struct _mulle_objc_infraclass *proto_infra)
{
   struct _mulle_objc_universe   *universe;

   assert( pair);
   assert( proto_infra);

   // adding the same protocol again is harmless and ignored
   // but don't search class hierarchy, so don't use conformsto

   assert( pair);

   if( ! _mulle_objc_classpair_has_protocolclass( pair, proto_infra))
   {
      _mulle_concurrent_pointerarray_add( &pair->protocolclasses, proto_infra);

      universe = _mulle_objc_classpair_get_universe( pair);
      if( universe->debug.trace.protocol_add || universe->debug.trace.dependency)
         mulle_objc_universe_trace( universe,
                                    "add protocolclass %08lx \"%s\" "
                                    "to class %08lx \"%s\"",
                                    (unsigned long) proto_infra->base.classid,
                                    proto_infra->base.name,
                                    (unsigned long) _mulle_objc_classpair_get_classid( pair),
                                    _mulle_objc_classpair_get_name( pair));

   }
}


mulle_objc_walkcommand_t
	_mulle_objc_classpair_walk_protocolclasses( struct _mulle_objc_classpair *pair,
                                               unsigned int inheritance,
                                               mulle_objc_walkprotocolclassescallback_t f,
                                               void *userinfo)
{
   mulle_objc_walkcommand_t                         rval;
   struct _mulle_objc_infraclass                    *proto_cls;
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_infraclass                    *infra;
   struct _mulle_objc_infraclass                    *superclass;
   struct _mulle_objc_classpair                     *superpair;

   rover = mulle_concurrent_pointerarray_enumerate( &pair->protocolclasses);
   while( proto_cls = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( rval = (*f)( proto_cls, pair, userinfo))
      {
         if( rval < mulle_objc_walk_ok)
            errno = ENOENT;
         return( rval);
      }
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      infra      = _mulle_objc_classpair_get_infraclass( pair);
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
      {
         superpair = _mulle_objc_infraclass_get_classpair( superclass);
         return( _mulle_objc_classpair_walk_protocolclasses( superpair, inheritance, f, userinfo));
      }
   }

   return( mulle_objc_walk_ok);
}


void   mulle_objc_classpair_add_protocolclassids_nofail( struct _mulle_objc_classpair *pair,
                                                         mulle_objc_protocolid_t *protocolclassids)
{
   struct _mulle_objc_infraclass   *proto_cls;
   struct _mulle_objc_universe     *universe;
   mulle_objc_protocolid_t         protocolclassid;
   mulle_objc_classid_t            classid;

   if( ! pair)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   if( ! protocolclassids)
      return;

   universe = _mulle_objc_classpair_get_universe( pair);
   classid  = _mulle_objc_classpair_get_classid( pair);
   while( (protocolclassid = *protocolclassids++) != MULLE_OBJC_NO_PROTOCOLID)
   {
      if( ! mulle_objc_uniqueid_is_sane( protocolclassid))
         _mulle_objc_classpair_fail_einval( pair);

      // if same as myself, no point in adding the protocolclass
      if( protocolclassid == classid)
         continue;

      // must already have this protocol
      if( ! _mulle_objc_classpair_has_protocolid( pair, protocolclassid))
         _mulle_objc_classpair_fail_einval( pair);

      proto_cls = _mulle_objc_universe_lookup_infraclass( universe, protocolclassid);
      if( ! proto_cls)
         _mulle_objc_classpair_fail_einval( pair);

      //
      // A class was assumed by the compiler to be a protocol class, but
      // it turns out it is not, since it's not a rootclass or has instance
      // variables or some-such, we warn and ignore, since the compiler can
      // not discern this for sure.
      //
      if( ! mulle_objc_infraclass_check_protocolclass( proto_cls))
         continue;

      if( _mulle_objc_classpair_has_protocolclass( pair, proto_cls))
         continue;

      if( _mulle_objc_infraclass_set_state_bit( proto_cls, MULLE_OBJC_INFRACLASS_IS_PROTOCOLCLASS))
      {
         if( universe->debug.trace.protocol_add)
            mulle_objc_universe_trace( universe,
                                       "class %08lx \"%s\" "
                                       "has become a protocolclass",
                                       (unsigned long) _mulle_objc_infraclass_get_classid( proto_cls),
                                       _mulle_objc_infraclass_get_name( proto_cls));
      }

      _mulle_objc_classpair_add_protocolclass( pair, proto_cls);
//      _mulle_concurrent_pointerarray_add( &pair->protocolclasses, proto_cls);
   }
}


struct _mulle_objc_infraclass  *
   _mulle_objc_protocolclassenumerator_next( struct _mulle_objc_protocolclassenumerator *rover)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *proto_cls;

   infra = _mulle_objc_protocolclassenumerator_get_infraclass( rover);
   while( proto_cls = _mulle_concurrent_pointerarrayenumerator_next( &rover->list_rover))
      if( proto_cls != infra)  // don't recurse into self
         break;
   return( proto_cls);
}


struct _mulle_objc_infraclass  *
   _mulle_objc_protocolclassreverseenumerator_next( struct _mulle_objc_protocolclassreverseenumerator *rover)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *proto_cls;

   infra = _mulle_objc_protocolclassreverseenumerator_get_infraclass( rover);
   while( proto_cls = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover->list_rover))
      if( proto_cls != infra)  // don't recurse into self
         break;
   return( proto_cls);
}


#pragma mark - protocols

mulle_objc_walkcommand_t
	_mulle_objc_classpair_walk_protocolids( struct _mulle_objc_classpair *pair,
                                           unsigned int inheritance,
                                           mulle_objc_walkprotocolidscallback_t f,
                                           void *userinfo)
{
   mulle_objc_walkcommand_t           rval;
   mulle_objc_categoryid_t            *p;
   mulle_objc_categoryid_t            *sentinel;
   struct _mulle_objc_uniqueidarray   *array;
   struct _mulle_objc_infraclass      *infra;
   struct _mulle_objc_infraclass      *superclass;
   struct _mulle_objc_classpair       *superpair;

   array    = _mulle_atomic_pointer_read( &pair->p_protocolids.pointer);
   p        = array->entries;
   sentinel = &p[ array->n];
   while( p < sentinel)
   {
      if( rval = (*f)( *p++, pair, userinfo))
      {
         if( rval < mulle_objc_walk_ok)
            errno = ENOENT;
         return( rval);
      }
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      infra      = _mulle_objc_classpair_get_infraclass( pair);
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
      {
         superpair = _mulle_objc_infraclass_get_classpair( superclass);
         return( _mulle_objc_classpair_walk_protocolids( superpair, inheritance, f, userinfo));
      }
   }
   return( mulle_objc_walk_ok);
}


int   __mulle_objc_classpair_conformsto_protocolid( struct _mulle_objc_classpair *pair,
                                                    unsigned int inheritance,
                                                    mulle_objc_protocolid_t protocolid)
{
   int                             rval;
   struct _mulle_objc_classpair    *superpair;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *superclass;

   rval = _mulle_objc_classpair_has_protocolid( pair, protocolid);
   if( rval)
      return( rval);

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      infra      = _mulle_objc_classpair_get_infraclass( pair);
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
      {
         superpair = _mulle_objc_infraclass_get_classpair( superclass);
         rval = _mulle_objc_classpair_conformsto_protocolid( superpair, protocolid);
         if( rval)
            return( rval);
      }
   }

   /* should query protocols too ? */

   return( rval);
}


#pragma mark - protocollist

void
  mulle_objc_classpair_add_protocollist_nofail( struct _mulle_objc_classpair *pair,
                                                struct _mulle_objc_protocollist *protocols)
{
   mulle_objc_protocolid_t       *q;
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_protocol   *p;
   struct _mulle_objc_protocol   *sentinel;

   if( ! pair)
      _mulle_objc_classpair_fail_einval( pair);

   if( ! protocols || ! protocols->n_protocols)
      return;

   universe = _mulle_objc_classpair_get_universe( pair);

   mulle_alloca_do( protocolids, mulle_objc_protocolid_t, protocols->n_protocols)
   {
      q        = protocolids;

      p        = protocols->protocols;
      sentinel = &p[ protocols->n_protocols];

      for(; p < sentinel; ++p)
      {
         if( ! mulle_objc_protocol_is_sane( p))
            _mulle_objc_classpair_fail_einval( pair);

         if( _mulle_objc_classpair_has_protocolid( pair, p->protocolid))
            continue;

         mulle_objc_universe_add_protocol_nofail( universe, p);

         if( universe->debug.trace.protocol_add)
            mulle_objc_universe_trace( universe,
                                       "add protocol %08lx \"%s\" to class "
                                       "%08lx \"%s\"",
                                       (unsigned long) p->protocolid,
                                       p->name,
                                       (unsigned long) _mulle_objc_classpair_get_classid( pair),
                                       _mulle_objc_classpair_get_name( pair));
         *q++ = p->protocolid;
      }

      _mulle_objc_classpair_add_protocolids( pair, (unsigned int) (q - protocolids), protocolids);
   }
}

