//
//  mulle_objc_classpair.c
//  mulle-objc
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
#include "mulle_objc_classpair.h"

#include <mulle_concurrent/mulle_concurrent.h>

#include "mulle_objc_class.h"
#include "mulle_objc_class_universe.h"
#include "mulle_objc_universe.h"



void    _mulle_objc_classpair_plusinit( struct _mulle_objc_classpair *pair,
                                        struct mulle_allocator *allocator)
{
   struct _mulle_objc_universe   *universe;
   
   universe = _mulle_objc_classpair_get_universe( pair);
   assert( universe);
   
   _mulle_concurrent_pointerarray_init( &pair->protocolclasses, 0, allocator);

   _mulle_atomic_pointer_nonatomic_write( &pair->p_protocolids.pointer, &universe->empty_uniqueidarray);
   _mulle_atomic_pointer_nonatomic_write( &pair->p_categoryids.pointer, &universe->empty_uniqueidarray);
}


void    _mulle_objc_classpair_plusdone( struct _mulle_objc_classpair *pair,
                                        struct mulle_allocator *allocator)
{
   struct _mulle_objc_uniqueidarray    *array;
   struct _mulle_objc_universe   *universe;
   
   universe = _mulle_objc_classpair_get_universe( pair);
   assert( universe);
   
   array = _mulle_atomic_pointer_nonatomic_read( &pair->p_protocolids.pointer);
   if( array != &universe->empty_uniqueidarray)
      mulle_objc_uniqueidarray_abafree( array, allocator);

   array = _mulle_atomic_pointer_nonatomic_read( &pair->p_categoryids.pointer);
   if( array != &universe->empty_uniqueidarray)
      mulle_objc_uniqueidarray_abafree( array, allocator);

   _mulle_concurrent_pointerarray_done( &pair->protocolclasses);
}


void   _mulle_objc_classpair_free( struct _mulle_objc_classpair *pair,
                                      struct mulle_allocator *allocator)
{
   assert( pair);
   assert( allocator);

   _mulle_objc_infraclass_plusdone( &pair->infraclass);
   _mulle_objc_class_done( _mulle_objc_infraclass_as_class( &pair->infraclass), allocator);
   _mulle_objc_metaclass_plusdone( &pair->metaclass);
   _mulle_objc_class_done( _mulle_objc_metaclass_as_class( &pair->metaclass), allocator);

   _mulle_objc_classpair_plusdone( pair, allocator);

   _mulle_allocator_free( allocator, pair);
}


void   mulle_objc_classpair_free( struct _mulle_objc_classpair *pair,
                                     struct mulle_allocator *allocator)
{
   if( ! pair)
      return;

   if( ! allocator)
   {
      errno = EINVAL;
      _mulle_objc_universe_raise_fail_errno_exception( mulle_objc_get_or_create_universe());
   }

   _mulle_objc_classpair_free( pair, allocator);
}


mulle_objc_walkcommand_t
   mulle_objc_classpair_walk( struct _mulle_objc_classpair *pair,
                              mulle_objc_walkcallback_t callback,
                              void *userinfo)
{
   mulle_objc_walkcommand_t   cmd;
   struct _mulle_objc_infraclass      *infra;
   struct _mulle_objc_metaclass       *meta;
   struct _mulle_objc_universe         *universe;

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

static void  _mulle_objc_classpair_add_uniqueidarray_ids( struct _mulle_objc_classpair *pair,
                                                          mulle_atomic_pointer_t *pointer,
                                                          unsigned int n,
                                                          mulle_objc_uniqueid_t *uniqueids)
{
   struct _mulle_objc_uniqueidarray   *array;
   struct _mulle_objc_uniqueidarray   *copy;
   struct mulle_allocator             *allocator;
   struct _mulle_objc_universe         *universe;
   
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
   while( ! _mulle_atomic_pointer_compare_and_swap( pointer, copy, array));
   
   if( array != &universe->empty_uniqueidarray)
      mulle_objc_uniqueidarray_abafree( array, allocator);
}


#pragma mark - categories

static inline void  _mulle_objc_classpair_add_categoryids( struct _mulle_objc_classpair *pair,
                                                           unsigned int n,
                                                           mulle_objc_protocolid_t *categoryids)
{
   _mulle_objc_classpair_add_uniqueidarray_ids( pair, &pair->p_categoryids.pointer, n, categoryids);
}


void   _mulle_objc_classpair_add_categoryid( struct _mulle_objc_classpair *pair,
                                             mulle_objc_categoryid_t categoryid)
{
   assert( pair);
   assert( categoryid != MULLE_OBJC_NO_CATEGORYID);
   assert( categoryid != MULLE_OBJC_INVALID_CATEGORYID);
   assert( ! _mulle_objc_classpair_has_categoryid( pair, categoryid));

   _mulle_objc_classpair_add_categoryids( pair, 1, &categoryid);
}


static void   _mulle_objc_class_raise_einval_exception( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_or_create_universe();
   errno = EINVAL;
   _mulle_objc_universe_raise_fail_errno_exception( universe);
}


int   _mulle_objc_classpair_walk_categoryids( struct _mulle_objc_classpair *pair,
                                              int (*f)( mulle_objc_categoryid_t,
                                                        struct _mulle_objc_classpair *,
                                                        void *),
                                              void *userinfo)
{
   int                                rval;
   mulle_objc_categoryid_t            *p;
   mulle_objc_categoryid_t            *sentinel;
   struct _mulle_objc_uniqueidarray   *array;
   
   array    = _mulle_atomic_pointer_read( &pair->p_categoryids.pointer);
   p        = array->entries;
   sentinel = &p[ array->n];
   while( p < sentinel)
   {
      if( rval = (*f)( *p++, pair, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   return( 0);
}


void   mulle_objc_classpair_unfailing_add_categoryid( struct _mulle_objc_classpair *pair,
                                                    mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_infraclass  *infra;

   if( ! pair)
      _mulle_objc_class_raise_einval_exception();

   if( categoryid == MULLE_OBJC_NO_CATEGORYID || categoryid == MULLE_OBJC_INVALID_CATEGORYID)
      _mulle_objc_class_raise_einval_exception();

   infra = _mulle_objc_classpair_get_infraclass( pair);
   universe = _mulle_objc_classpair_get_universe( pair);


   // adding a category twice is very bad
   if( _mulle_objc_classpair_has_categoryid( pair, categoryid))
   {
      fprintf( stderr, "mulle_objc_universe %p error: category %08x for"
                       " class %08x \"%s\" is already loaded\n",
              universe,
              categoryid,
              _mulle_objc_classpair_get_classid( pair),
              _mulle_objc_classpair_get_name( pair));
      _mulle_objc_class_raise_einval_exception();
   }

   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRA_IS_PROTOCOLCLASS))
   {
      if( universe->debug.warn.protocolclass)
         if( universe->foundation.rootclassid != _mulle_objc_classpair_get_classid( pair))
            fprintf( stderr, "mulle_objc_universe %p warning: class %08x \"%s\""
                             " is a protocolclass and gains a"
                             " category %08x \"%s( %s)\"\n",
                    universe,
                    _mulle_objc_classpair_get_classid( pair),
                    _mulle_objc_classpair_get_name( pair),
                    categoryid,
                    _mulle_objc_classpair_get_name( pair),
                    mulle_objc_string_for_categoryid( categoryid));
   }

   if( universe->debug.trace.category_adds || universe->debug.trace.dependencies)
      fprintf( stderr, "mulle_objc_universe %p trace: add category %08x \"%s\""
                       " to class %08x \"%s\"\n",
              universe,
              categoryid,
              mulle_objc_string_for_categoryid( categoryid),
              _mulle_objc_classpair_get_classid( pair),
              _mulle_objc_classpair_get_name( pair));

   _mulle_objc_classpair_add_categoryid( pair, categoryid);
}


#pragma mark - protocolclasses

void   _mulle_objc_classpair_add_protocolclass( struct _mulle_objc_classpair *pair,
                                                struct _mulle_objc_infraclass *proto_cls)
{
   assert( pair);
   assert( proto_cls);

   // adding the same protocol again is harmless and ignored
   // but don't search class hierarchy, so don't use conformsto

   if( ! _mulle_objc_classpair_has_protocolclass( pair, proto_cls))
      _mulle_concurrent_pointerarray_add( &pair->protocolclasses, proto_cls);
}


int   _mulle_objc_classpair_walk_protocolclasses( struct _mulle_objc_classpair *pair,
                                                  int (*f)( struct _mulle_objc_infraclass *,
                                                        struct _mulle_objc_classpair *,
                                                        void *) ,
                                                  void *userinfo)
{
   int                                              rval;
   struct _mulle_objc_infraclass                    *proto_cls;
   struct mulle_concurrent_pointerarrayenumerator   rover;

   rover = mulle_concurrent_pointerarray_enumerate( &pair->protocolclasses);
   while( proto_cls = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( rval = (*f)( proto_cls, pair, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   return( 0);
}


void   mulle_objc_classpair_unfailing_add_protocolclassids( struct _mulle_objc_classpair *pair,
                                                            mulle_objc_protocolid_t *protocolclassids)
{
   mulle_objc_protocolid_t          protocolclassid;
   struct _mulle_objc_infraclass    *proto_cls;
   struct _mulle_objc_universe       *universe;
   mulle_objc_classid_t             classid;

   if( ! pair)
      _mulle_objc_class_raise_einval_exception();

   if( ! protocolclassids)
      return;

   universe = _mulle_objc_classpair_get_universe( pair);
   classid = _mulle_objc_classpair_get_classid( pair);

   while( protocolclassid = *protocolclassids++)
   {
      if( protocolclassid == MULLE_OBJC_NO_PROTOCOLID || protocolclassid == MULLE_OBJC_INVALID_PROTOCOLID)
         _mulle_objc_class_raise_einval_exception();

      // if same as myself, no point in adding the protocolclass
      if( protocolclassid == classid)
         continue;

      // must already have this protocol
      if( ! _mulle_objc_classpair_has_protocolid( pair, protocolclassid))
         _mulle_objc_class_raise_einval_exception();

      proto_cls = _mulle_objc_universe_get_or_lookup_infraclass( universe, protocolclassid);
      if( ! proto_cls)
         _mulle_objc_class_raise_einval_exception();

      if( ! mulle_objc_infraclass_is_protocolclass( proto_cls))
         _mulle_objc_class_raise_einval_exception();

      if( _mulle_objc_classpair_has_protocolclass( pair, proto_cls))
         continue;

      if( _mulle_objc_infraclass_set_state_bit( proto_cls, MULLE_OBJC_INFRA_IS_PROTOCOLCLASS))
      {
         if( universe->debug.trace.protocol_adds)
            fprintf( stderr, "mulle_objc_universe %p trace: class %08x \"%s\""
                             " has become a protocolclass\n",
                 universe,
                 _mulle_objc_infraclass_get_classid( proto_cls),
                 _mulle_objc_infraclass_get_name( proto_cls));
      }

      if( universe->debug.trace.protocol_adds)
         fprintf( stderr, "mulle_objc_universe %p trace: add protocolclass %08x"
                          " \"%s\" to class %08x \"%s\"\n",
                 universe,
                 protocolclassid,
                 _mulle_objc_infraclass_get_name( proto_cls),
                 _mulle_objc_classpair_get_classid( pair),
                 _mulle_objc_classpair_get_name( pair));

      _mulle_concurrent_pointerarray_add( &pair->protocolclasses, proto_cls);
   }
}


struct _mulle_objc_infraclass  *_mulle_objc_protocolclassenumerator_next( struct _mulle_objc_protocolclassenumerator *rover)
{
   struct _mulle_objc_infraclass    *infra;
   struct _mulle_objc_infraclass    *proto_cls;

   infra = _mulle_objc_protocolclassenumerator_get_infraclass( rover);
   while( proto_cls = _mulle_concurrent_pointerarrayenumerator_next( &rover->list_rover))
      if( proto_cls != infra)  // don't recurse into self
         break;
   return( proto_cls);
}


#pragma mark - protocols


static inline void  _mulle_objc_classpair_add_protocolids( struct _mulle_objc_classpair *pair,
                                                           unsigned int n,
                                                           mulle_objc_protocolid_t *protocolids)
{
   _mulle_objc_classpair_add_uniqueidarray_ids( pair, &pair->p_protocolids.pointer, n, protocolids);
}



int   _mulle_objc_classpair_walk_protocolids( struct _mulle_objc_classpair *pair,
                                              int (*f)( mulle_objc_protocolid_t,
                                                        struct _mulle_objc_classpair *,
                                                        void *),
                                              void *userinfo)
{
   int                                rval;
   mulle_objc_categoryid_t            *p;
   mulle_objc_categoryid_t            *sentinel;
   struct _mulle_objc_uniqueidarray   *array;
   
   array    = _mulle_atomic_pointer_read( &pair->p_protocolids.pointer);
   p        = array->entries;
   sentinel = &p[ array->n];
   while( p < sentinel)
   {
      if( rval = (*f)( *p++, pair, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   return( 0);
}


int   _mulle_objc_classpair_conformsto_protocolid( struct _mulle_objc_classpair *pair,
                                                   mulle_objc_protocolid_t protocolid)
{
   int                             rval;
   struct _mulle_objc_classpair    *superpair;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *superclass;

   rval = _mulle_objc_classpair_has_protocolid( pair, protocolid);
   if( rval)
      return( rval);

   infra = _mulle_objc_classpair_get_infraclass( pair);
   if( ! (_mulle_objc_infraclass_get_inheritance( infra) & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
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

void   mulle_objc_classpair_unfailing_add_protocollist( struct _mulle_objc_classpair *pair,
                                                        struct _mulle_objc_protocollist *protocols)
{
   mulle_objc_protocolid_t       *q;
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_protocol   *p;
   struct _mulle_objc_protocol   *sentinel;
   
   if( ! pair)
      _mulle_objc_class_raise_einval_exception();

   if( ! protocols || ! protocols->n_protocols)
      return;

   universe = _mulle_objc_classpair_get_universe( pair);

   {
      auto mulle_objc_protocolid_t    protocolids[ protocols->n_protocols];
   
      p        = protocols->protocols;
      sentinel = &p[ protocols->n_protocols];
      q        = protocolids;
      
      for(; p < sentinel; ++p)
      {
         if( ! mulle_objc_protocol_is_sane( p))
            _mulle_objc_class_raise_einval_exception();

         if( _mulle_objc_classpair_has_protocolid( pair, p->protocolid))
            continue;
         
         mulle_objc_universe_unfailing_add_protocol( universe, p);
         
         if( universe->debug.trace.protocol_adds)
            fprintf( stderr, "mulle_objc_universe %p trace: add protocol %08x \"%s\" to class %08x \"%s\"\n",
                    universe,
                    p->protocolid,
                    p->name,
                    _mulle_objc_classpair_get_classid( pair),
                    _mulle_objc_classpair_get_name( pair));
         *q++ = p->protocolid;
      }
      
      _mulle_objc_classpair_add_protocolids( pair, (unsigned int) (q - protocolids), protocolids);
   }
}

