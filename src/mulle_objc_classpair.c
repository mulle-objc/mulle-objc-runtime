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
#include "mulle_objc_class_runtime.h"
#include "mulle_objc_runtime.h"


void    _mulle_objc_classpair_plusinit( struct _mulle_objc_classpair *pair,
                                               struct mulle_allocator *allocator)
{
   _mulle_concurrent_pointerarray_init( &pair->protocolids, 0, allocator);
   _mulle_concurrent_pointerarray_init( &pair->categoryids, 0, allocator);
}


void    _mulle_objc_classpair_plusdone( struct _mulle_objc_classpair *pair)
{
   _mulle_concurrent_pointerarray_done( &pair->protocolids);
   _mulle_concurrent_pointerarray_done( &pair->categoryids);
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

   _mulle_objc_classpair_plusdone( pair);

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
      _mulle_objc_runtime_raise_fail_errno_exception( __get_or_create_objc_runtime());
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
   struct _mulle_objc_runtime         *runtime;

   runtime = _mulle_objc_classpair_get_runtime( pair);

   cmd = (*callback)( runtime, pair, mulle_objc_walkpointer_is_classpair, NULL, NULL, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   infra = _mulle_objc_classpair_get_infraclass( pair);
   cmd   = mulle_objc_infraclass_walk( infra, mulle_objc_walkpointer_is_infraclass,
                                       callback, runtime, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   meta = _mulle_objc_classpair_get_metaclass( pair);
   cmd  = mulle_objc_metaclass_walk( meta, mulle_objc_walkpointer_is_metaclass,
                                     callback, infra, userinfo);
   return( cmd);
}




#pragma mark - categories

int   _mulle_objc_classpair_walk_categoryids( struct _mulle_objc_classpair *pair,
                                              int (*f)( mulle_objc_categoryid_t,
                                                        struct _mulle_objc_classpair *,
                                                        void *),
                                              void *userinfo)
{
   int                                              rval;
   mulle_objc_categoryid_t                          categoryid;
   struct mulle_concurrent_pointerarrayenumerator   rover;
   void                                             *value;

   rover = mulle_concurrent_pointerarray_enumerate( &pair->categoryids);
   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      categoryid = (mulle_objc_categoryid_t) (uintptr_t) value; // fing warning
      if( rval = (*f)( categoryid, pair, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   return( 0);
}


#pragma mark - protocols

void   _mulle_objc_classpair_add_protocol( struct _mulle_objc_classpair *pair,
                                           mulle_objc_protocolid_t protocolid)
{
   assert( pair);
   assert( protocolid != MULLE_OBJC_NO_PROTOCOLID);
   assert( protocolid != MULLE_OBJC_INVALID_PROTOCOLID);

   // adding the same protocol again is harmless and ignored
   // but don't search class hierarchy, so don't use conformsto

   if( ! _mulle_objc_classpair_has_protocol( pair, protocolid))
      _mulle_concurrent_pointerarray_add( &pair->protocolids, (void *) (uintptr_t) protocolid);
}


int   _mulle_objc_classpair_walk_protocolids( struct _mulle_objc_classpair *pair,
                                              int (*f)( mulle_objc_protocolid_t,
                                                        struct _mulle_objc_classpair *,
                                                        void *) ,
                                              void *userinfo)
{
   int                                         rval;
   mulle_objc_propertyid_t                     propertyid;
   struct mulle_concurrent_pointerarrayenumerator   rover;
   void                                        *value;

   rover = mulle_concurrent_pointerarray_enumerate( &pair->protocolids);
   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      propertyid = (mulle_objc_propertyid_t) (uintptr_t) value; // fing warning
      if( rval = (*f)( propertyid, pair, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   return( 0);
}


static void   _mulle_objc_class_raise_einval_exception( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = __get_or_create_objc_runtime();
   errno = EINVAL;
   _mulle_objc_runtime_raise_fail_errno_exception( runtime);
}


void   mulle_objc_classpair_unfailing_add_category( struct _mulle_objc_classpair *pair,
                                                    mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_runtime    *runtime;

   if( ! pair)
      _mulle_objc_class_raise_einval_exception();

   if( categoryid == MULLE_OBJC_NO_CATEGORYID || categoryid == MULLE_OBJC_INVALID_CATEGORYID)
      _mulle_objc_class_raise_einval_exception();

   // adding a category twice is very bad
   if( _mulle_objc_classpair_has_category( pair, categoryid))
      _mulle_objc_class_raise_einval_exception();

   runtime = _mulle_objc_classpair_get_runtime( pair);
   if( runtime->debug.trace.category_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add category %08x \"%s\" to class %08x \"%s\"\n",
              runtime,
              categoryid,
              mulle_objc_string_for_categoryid( categoryid),
              _mulle_objc_classpair_get_classid( pair),
              _mulle_objc_classpair_get_name( pair));

   _mulle_objc_classpair_add_category( pair, categoryid);
}



void   mulle_objc_classpair_unfailing_add_protocols( struct _mulle_objc_classpair *pair,
                                                     mulle_objc_protocolid_t *protocolids)
{
   mulle_objc_protocolid_t        protocolid;
   struct _mulle_objc_runtime     *runtime;

   if( ! pair)
      _mulle_objc_class_raise_einval_exception();

   if( ! protocolids)
      return;

   runtime = _mulle_objc_classpair_get_runtime( pair);

   while( protocolid = *protocolids++)
   {
      if( protocolid == MULLE_OBJC_NO_PROTOCOLID || protocolid == MULLE_OBJC_INVALID_PROTOCOLID)
         _mulle_objc_class_raise_einval_exception();

      if( _mulle_objc_classpair_has_protocol( pair, protocolid))
         continue;

      if( runtime->debug.trace.protocol_adds)
         fprintf( stderr, "mulle_objc_runtime %p trace: add protocol %08x \"%s\" to class %08x \"%s\"\n",
                 runtime,
                 protocolid,
                 mulle_objc_string_for_protocolid( protocolid),
                 _mulle_objc_classpair_get_classid( pair),
                 _mulle_objc_classpair_get_name( pair));

      _mulle_objc_classpair_add_protocol( pair, protocolid);
   }
}


int   _mulle_objc_classpair_conformsto_protocol( struct _mulle_objc_classpair *pair,
                                                 mulle_objc_protocolid_t protocolid)
{
   int                             rval;
   struct _mulle_objc_classpair    *superpair;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *superclass;

   rval = _mulle_objc_classpair_has_protocol( pair, protocolid);
   if( rval)
      return( rval);

   infra = _mulle_objc_classpair_get_infraclass( pair);
   if( ! (_mulle_objc_infraclass_get_inheritance( infra) & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass && superclass != infra)
      {
         superpair = _mulle_objc_infraclass_get_classpair( superclass);
         rval = _mulle_objc_classpair_conformsto_protocol( superpair, protocolid);
         if( rval)
            return( rval);
      }
   }

   /* should query protocols too ? */

   return( rval);
}


struct _mulle_objc_infraclass  *_mulle_objc_protocolclassenumerator_next( struct _mulle_objc_protocolclassenumerator *rover)
{
   struct _mulle_objc_classpair     *pair;
   struct _mulle_objc_infraclass    *infra;
   struct _mulle_objc_infraclass    *proto_cls;
   struct _mulle_objc_runtime       *runtime;
   mulle_objc_protocolid_t          uniqueid;
   void                             *value;

   proto_cls  = NULL;

   pair    = _mulle_objc_protocolclassenumerator_get_classpair( rover);
   runtime = _mulle_objc_classpair_get_runtime( pair);
   infra   = _mulle_objc_classpair_get_infraclass( pair);

   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover->list_rover))
   {
      uniqueid = (mulle_objc_protocolid_t) (uintptr_t) value;
      if( _mulle_objc_infraclass_get_classid( infra) == uniqueid)  // don't recurse into self
         continue;

      proto_cls = _mulle_objc_runtime_lookup_infraclass( runtime, uniqueid);
      if( ! mulle_objc_infraclass_is_protocol_class( proto_cls))
         continue;
   }
   return( proto_cls);
}

