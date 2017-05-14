//
//  mulle_objc_class.c
//  mulle-objc
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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
#include "mulle_objc_class.h"

#include "mulle_objc_classpair.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_ivar.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_call.h"
#include "mulle_objc_callqueue.h"
#include "mulle_objc_kvccache.h"
#include "mulle_objc_metaclass.h"
#include "mulle_objc_method.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_taggedpointer.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_concurrent/mulle_concurrent.h>


// public but not publizied

void   *_mulle_objc_call_class_needs_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls);
void   *mulle_objc_call_needs_cache2( void *obj, mulle_objc_methodid_t methodid, void *parameter);


# pragma mark - accessor

static char   *lookup_bitname( unsigned int bit)
{
   // some "known" values
   switch( bit)
   {
   case MULLE_OBJC_CACHE_INITIALIZED      : return( "CACHE_INITIALIZED");
   case MULLE_OBJC_ALWAYS_EMPTY_CACHE     : return( "EMPTY_CACHE");
   case 0x4                               : return( "WARN_PROTOCOL");
   case 0x8                               : return( "IS_PROTOCOLCLASS");
   case 0x10                              : return( "LOAD_SCHEDULED");
   case 0x20                              : return( "INITIALIZE_DONE");
   }
   return( 0);
}


int   _mulle_objc_class_set_state_bit( struct _mulle_objc_class *cls, unsigned int bit)
{
   void   *state;
   void   *old;
   struct _mulle_objc_runtime   *runtime;
   char   *bitname;

   assert( bit);

   do
   {
      old   = _mulle_atomic_pointer_read( &cls->state);
      state = (void *) ((uintptr_t) old | bit);
      if( state == old)
         return( 0);
   }
   while( ! _mulle_atomic_pointer_compare_and_swap( &cls->state, state, old));

   runtime = _mulle_objc_class_get_runtime( cls);
   if( runtime->debug.trace.state_bits)
   {
      bitname = lookup_bitname( bit);
      fprintf( stderr, "mulle_objc_runtime %p trace: %s %08x \"%s\" gained the 0x%x bit (%s)\n",
              runtime, _mulle_objc_class_get_classtypename( cls), cls->classid, cls->name, bit, bitname ? bitname : "???");
   }
   return( 1);
}


# pragma mark - calls


static void   *_mulle_objc_call_class_waiting_for_cache( void *obj,
                                                         mulle_objc_methodid_t methodid,
                                                         void *parameter,
                                                         struct _mulle_objc_class *cls)
{
   /* same thread ? we are single threaded! */
   if( _mulle_atomic_pointer_read( &cls->thread) != (void *) mulle_thread_self())
   {
      /* wait for other thread to finish with +initialize */
      /* TODO: using yield is poor though! Use a condition to be awaken! */
      while( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED))
         mulle_thread_yield();
   }

   return( mulle_objc_object_call_uncached_class( obj, methodid, parameter, cls));
}


void   *_mulle_objc_call_class_needs_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_infraclass    *infra;
   struct _mulle_objc_metaclass     *meta;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_method        *initialize;
   mulle_objc_cache_uint_t          n_entries;

   assert( mulle_objc_class_is_current_thread_registered( cls));

   //
   // An uninitialized class has the empty_cache as the cache. It also has
   // `cls->thread` NULL. This methods is therefore usually called twice
   // once for the meta class and once for the instance. Regardless in both
   // cases, it is checked if +initialize needs to run. But this is only
   // flagged in the meta class.
   //
   // If another thread enters here, it will expect `cls->thread` to be NULL.
   // If it isn't it waits for MULLE_OBJC_CACHE_INITIALIZED to go up.
   //
   // what is tricky is, that cls and metaclass are executing this
   // singlethreaded, but still cls and metaclass could be in different threads
   //

   if( ! _mulle_atomic_pointer_compare_and_swap( &cls->thread, (void *) mulle_thread_self(), NULL))
      return( _mulle_objc_call_class_waiting_for_cache( obj, methodid, parameter, cls));

   // Singlethreaded block with respect to cls, not meta though!
   {
      struct _mulle_objc_runtime   *runtime;

      runtime = _mulle_objc_class_get_runtime( cls);

      //
      // first do +initialize,  uncached execution
      // track state only in "meta" class
      //
      if( _mulle_objc_class_is_infraclass( cls))
      {
         infra = (struct _mulle_objc_infraclass *) cls;
         meta  = _mulle_objc_infraclass_get_metaclass( infra);
      }
      else
      {
         meta  = (struct _mulle_objc_metaclass *) cls;
         infra = _mulle_objc_metaclass_get_infraclass( meta);
      }

      if( _mulle_objc_metaclass_set_state_bit( meta, MULLE_OBJC_META_INITIALIZE_DONE))
      {
         // grab code from superclass
         // this is useful for MulleObjCSingleton
         initialize = _mulle_objc_class_search_method( &meta->base, MULLE_OBJC_INITIALIZE_METHODID, NULL, MULLE_OBJC_ANY_OWNER, meta->base.inheritance);
         if( initialize)
         {
            if( runtime->debug.trace.initialize)
               fprintf( stderr, "mulle_objc_runtime %p trace: call +[%s initialize]\n", runtime, cls->name);

            (*_mulle_objc_method_get_implementation( initialize))( (struct _mulle_objc_object *) infra, MULLE_OBJC_INITIALIZE_METHODID, NULL);
         }
         else
            if( runtime->debug.trace.initialize)
               fprintf( stderr, "mulle_objc_runtime %p trace: no +[%s initialize] found\n", runtime, cls->name);
      }

      // now setup the cache and let it rip, except when we don't ever want one
      if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_ALWAYS_EMPTY_CACHE))
      {
         n_entries = _mulle_objc_class_convenient_methodcache_size( cls);
         cache     = mulle_objc_cache_new( n_entries, &cls->runtime->memory.allocator);

         assert( cache);
         assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries) == mulle_objc_get_runtime()->empty_cache.entries);

         _mulle_atomic_pointer_nonatomic_write( &cls->cachepivot.pivot.entries, cache->entries);
         cls->cachepivot.call2 = mulle_objc_object_call2;
         cls->call             = mulle_objc_object_call_class;

         if( runtime->debug.trace.method_caches)
            fprintf( stderr, "mulle_objc_runtime %p trace: added cache to %s %08x \"%s\" (%p) with %u entries\n",
                  runtime,
                     _mulle_objc_class_get_classtypename( cls),
                     _mulle_objc_class_get_classid( cls),
                     _mulle_objc_class_get_name( cls),
                     cls,
                     cache->size);
      }
      else
      {
         cls->cachepivot.call2 = mulle_objc_object_call2_empty_cache;
         cls->call             = mulle_objc_object_call_class_empty_cache;

         if( runtime->debug.trace.method_caches)
            fprintf( stderr, "mulle_objc_runtime %p trace: using always empty cache on %s %08x \"%s\" (%p)\n",
                  runtime,
                     _mulle_objc_class_get_classtypename( cls),
                     _mulle_objc_class_get_classid( cls),
                     _mulle_objc_class_get_name( cls),
                     cls);
      }

      // finally unfreze
      // threads waiting_for_cache will run now

      _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED);
   }

   //
   // count #caches, if there are zero caches yet, the runtime can be much
   // faster adding methods.
   //
   _mulle_atomic_pointer_increment( &cls->runtime->cachecount_1);

   return( (*cls->call)( obj, methodid, parameter, cls));
}


void   *mulle_objc_call_needs_cache2( void *obj, mulle_objc_methodid_t methodid, void *parameter)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( (struct _mulle_objc_object *) obj);
   return( _mulle_objc_call_class_needs_cache( obj, methodid, parameter, cls));
}


# pragma mark - initialization / deallocation

void   _mulle_objc_class_init( struct _mulle_objc_class *cls,
                               char *name,
                               size_t  instancesize,
                               mulle_objc_classid_t classid,
                               struct _mulle_objc_class *superclass,
                               struct _mulle_objc_runtime *runtime)
{
   extern void   *mulle_objc_call_needs_cache2( void *obj, mulle_objc_methodid_t methodid, void *parameter);

   assert( runtime);

   cls->name                     = name;

   cls->superclass               = superclass;
   cls->superclassid             = superclass ? superclass->classid : MULLE_OBJC_NO_CLASSID;
   cls->classid                  = classid;

   cls->allocationsize           = sizeof( struct _mulle_objc_objectheader) + instancesize;
   cls->call                     = _mulle_objc_call_class_needs_cache;
   cls->runtime                  = runtime;
   cls->inheritance              = runtime->classdefaults.inheritance;

   cls->cachepivot.call2         = mulle_objc_call_needs_cache2;

   _mulle_atomic_pointer_nonatomic_write( &cls->cachepivot.pivot.entries, runtime->empty_cache.entries);
   _mulle_atomic_pointer_nonatomic_write( &cls->kvc.entries, runtime->empty_cache.entries);

   _mulle_concurrent_pointerarray_init( &cls->methodlists, 0, &runtime->memory.allocator);

   _mulle_objc_fastmethodtable_init( &cls->vtab);
}


void   _mulle_objc_class_done( struct _mulle_objc_class *cls,
                               struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache      *cache;

   assert( cls);
   assert( allocator);

   _mulle_objc_fastmethodtable_done( &cls->vtab);

   _mulle_concurrent_pointerarray_done( &cls->methodlists);

   _mulle_objc_class_invalidate_all_kvcinfos( cls);

   cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   if( cache != &cls->runtime->empty_cache)
      _mulle_objc_cache_free( cache, allocator);
}


# pragma mark - forwarding

static inline struct _mulle_objc_method   *_mulle_objc_class_search_forwardmethod( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_search_method( cls, MULLE_OBJC_FORWARD_METHODID, NULL, MULLE_OBJC_ANY_OWNER, cls->inheritance);
   if( ! method)
      method = cls->runtime->classdefaults.forwardmethod;

   return( method);
}


struct _mulle_objc_method    *_mulle_objc_class_getorsearch_forwardmethod( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   assert( mulle_objc_class_is_current_thread_registered( cls));

   method = _mulle_objc_class_get_forwardmethod( cls);
   if( ! method)
   {
      method = _mulle_objc_class_search_forwardmethod( cls);
      if( method)
         _mulle_objc_class_set_forwardmethod( cls, method);
   }
   return( method);
}

MULLE_C_NO_RETURN
static void  _mulle_objc_class_raise_method_not_found( struct _mulle_objc_class *cls,
                   mulle_objc_methodid_t missing_method)
{
   char   *prefix;
   char   *name;

   prefix = _mulle_objc_class_is_metaclass( cls) ? "meta-" : "";
   name   = _mulle_objc_class_get_name( cls);

   if( errno != ENOENT)
      _mulle_objc_runtime_raise_inconsistency_exception( cls->runtime, "mulle_objc_runtime: forward:: method has wrong id in %sclass \"%s\"",
                                                        prefix,
                                                        name);
   if( missing_method)
      _mulle_objc_class_raise_method_not_found_exception( cls, missing_method);

   _mulle_objc_runtime_raise_inconsistency_exception( cls->runtime, "mulle_objc_runtime: missing forward:: method in %sclass \"%s\"",
                                                     prefix,
                                                     name);
}


MULLE_C_NON_NULL_RETURN
struct _mulle_objc_method    *_mulle_objc_class_unfailing_getorsearch_forwardmethod( struct _mulle_objc_class *cls,
                                                                               mulle_objc_methodid_t   missing_method)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_getorsearch_forwardmethod( cls);
   if( method)
      return( method);

   _mulle_objc_class_raise_method_not_found( cls, missing_method);
}


# pragma mark - consistency check

static int  _mulle_objc_class_methodlists_are_sane( struct _mulle_objc_class *cls)
{
   void   *storage;

   // need at least one possibly empty message list
   storage = _mulle_atomic_pointer_nonatomic_read( &cls->methodlists.storage.pointer);
   if( ! storage || ! mulle_concurrent_pointerarray_get_count( &cls->methodlists))
   {
      errno = ECHILD;
      return( 0);
   }

   return( 1);
}

# pragma mark - consistency

int   __mulle_objc_class_is_sane( struct _mulle_objc_class *cls)
{
   //
   // just check for some glaring errors
   //
   if( ! cls || (cls->classid == MULLE_OBJC_NO_CLASSID) || (cls->classid == MULLE_OBJC_INVALID_CLASSID))
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! cls->name || ! strlen( cls->name))
   {
      errno = EINVAL;
      return( 0);
   }

   //
   // make sure the alignment is not off, so it might be treated as a
   // tagged pointer
   //
   if( mulle_objc_taggedpointer_get_index( cls))
   {
      errno = EACCES;
      return( 0);
   }

   assert( cls->call);
   assert( cls->cachepivot.call2);
   assert( mulle_objc_classid_from_string( cls->name) == cls->classid);

   return( 1);
}


int   _mulle_objc_class_is_sane( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_metaclass   *meta;

   if( ! __mulle_objc_class_is_sane( cls))
      return( 0);

   if( ! _mulle_objc_class_methodlists_are_sane( cls))
      return( 0);

   meta = _mulle_objc_class_get_metaclass( cls);
   if( ! meta)
   {
      errno = ECHILD;
      return( 0);
   }

   return( 1);
}


# pragma mark - methodlists

static int   _mulle_objc_class_invalidate_methodcache( struct _mulle_objc_class *cls,
                                                       mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_runtime      *runtime;

   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   cache = _mulle_objc_class_get_methodcache( cls);
   if( ! _mulle_atomic_pointer_read( &cache->n))
      return( 0);

   offset = _mulle_objc_cache_offset_for_uniqueid( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   // no entry is matching, fine
   if( ! entry->key.uniqueid)
      return( 0);

   //
   // just swap out the current cache, place a fresh cache in there
   //
   _mulle_objc_class_add_entry_by_swapping_caches( cls, cache, NULL, MULLE_OBJC_NO_UNIQUEID);

   runtime = _mulle_objc_class_get_runtime( cls);
   if( runtime->debug.trace.method_caches)
      fprintf( stderr, "mulle_objc_runtime %p trace: invalidate cache of %s %08x \"%s\" (%p)\n",
                  runtime,
                     _mulle_objc_class_get_classtypename( cls),
                     _mulle_objc_class_get_classid( cls),
                     _mulle_objc_class_get_name( cls),
                     cls);
   return( 0x1);
}


static int  invalidate_caches( struct _mulle_objc_runtime *runtime,
                               struct _mulle_objc_class *cls,
                               enum mulle_objc_walkpointertype_t type,
                               char *key,
                               void *parent,
                               struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_methodlistenumerator   rover;
   struct _mulle_objc_method                 *method;

   // preferably nothing there yet
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED))
      return( mulle_objc_walk_ok);

   _mulle_objc_class_invalidate_all_kvcinfos( cls);

   // if caches have been cleaned for class, it's done
   rover = _mulle_objc_methodlist_enumerate( list);
   while( method = _mulle_objc_methodlistenumerator_next( &rover))
      if( _mulle_objc_class_invalidate_methodcache( cls, method->descriptor.methodid))
         break;
   _mulle_objc_methodlistenumerator_done( &rover);

   return( mulle_objc_walk_ok);
}


void   mulle_objc_class_did_add_methodlist( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_methodlist *list)
{
   //
   // now walk through the method list again
   // and update all caches, that need it
   // this is SLOW
   //
   if( list && list->n_methods)
   {
      //
      // this optimization works as long as you are installing plain classes.
      //
      if( _mulle_atomic_pointer_read( &cls->runtime->cachecount_1))
         mulle_objc_runtime_walk_classes( cls->runtime, (int (*)()) invalidate_caches, list);
   }
}


int   _mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_methodlistenumerator   rover;
   struct _mulle_objc_method                 *method;
   mulle_objc_uniqueid_t                     last;
   unsigned int                              n;

   if( ! list)
      list = &cls->runtime->empty_methodlist;

   /* register instance methods */
   n     = 0;
   last  = MULLE_OBJC_MIN_UNIQUEID - 1;
   rover = _mulle_objc_methodlist_enumerate( list);
   while( method = _mulle_objc_methodlistenumerator_next( &rover))
   {
      assert( method->descriptor.methodid != MULLE_OBJC_NO_METHODID && method->descriptor.methodid != MULLE_OBJC_INVALID_METHODID);
      //
      // methods must be sorted by signed methodid, so we can binary search them
      // (in the future)
      //
      if( last > method->descriptor.methodid)
      {
         errno = EDOM;
         return( -1);
      }

      last = method->descriptor.methodid;
      if( _mulle_objc_runtime_add_methoddescriptor( cls->runtime, &method->descriptor))
      {
         _mulle_objc_methodlistenumerator_done( &rover);
         errno = EEXIST;
         return( -1);
      }

      if( _mulle_objc_methoddescriptor_is_preload_method( &method->descriptor))
      {
         cls->preloads++;
      }
      ++n;
   }
   _mulle_objc_methodlistenumerator_done( &rover);

   _mulle_concurrent_pointerarray_add( &cls->methodlists, list);
   return( 0);
}


int   mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                       struct _mulle_objc_methodlist *list)
{
   int  rval;

   if( ! cls)
   {
      errno = EINVAL;
      return( -1);
   }

   rval = _mulle_objc_class_add_methodlist( cls, list);
   if( ! rval)
      mulle_objc_class_did_add_methodlist( cls, list);
   return( rval);
}


void   mulle_objc_class_unfailing_add_methodlist( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_methodlist *list)
{
   if( mulle_objc_class_add_methodlist( cls, list))
      _mulle_objc_runtime_raise_fail_errno_exception( cls->runtime);
}



static int   _mulle_objc_class_protocol_walk_methods( struct _mulle_objc_class *cls,
                                                      unsigned int inheritance,
                                                      int (*f)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *),
                                                      void *userinfo)
{
   int                                          rval;
   struct _mulle_objc_class                     *walk_cls;
   struct _mulle_objc_infraclass                *infra;
   struct _mulle_objc_infraclass                *proto_cls;
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_protocolclassenumerator   rover;
   int                                          is_meta;

   rval    = 0;
   pair    = _mulle_objc_class_get_classpair( cls);
   infra   = _mulle_objc_classpair_get_infraclass( pair);
   is_meta = _mulle_objc_class_is_metaclass( cls);

   rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   while( proto_cls = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      if( proto_cls == infra)
         continue;

      walk_cls = _mulle_objc_infraclass_as_class( proto_cls);
      if( is_meta)
         walk_cls = _mulle_objc_metaclass_as_class( _mulle_objc_infraclass_get_metaclass( proto_cls));

      if( rval = _mulle_objc_class_walk_methods( walk_cls, inheritance | walk_cls->inheritance, f, userinfo))
         break;
   }
   _mulle_objc_protocolclassenumerator_done( &rover);

   return( rval);
}


static struct _mulle_objc_method  *_mulle_objc_class_protocol_search_method( struct _mulle_objc_class *cls,
                                                                             mulle_objc_methodid_t methodid,
                                                                             struct _mulle_objc_method *previous,
                                                                             void *owner,
                                                                             unsigned int inheritance)
{
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_infraclass                *infra;
   struct _mulle_objc_class                     *walk_cls;
   struct _mulle_objc_infraclass                *proto_cls;
   struct _mulle_objc_protocolclassenumerator   rover;
   struct _mulle_objc_method                    *found;
   struct _mulle_objc_method                    *method;
   int                                          is_meta;

   pair   = _mulle_objc_class_get_classpair( cls);
   infra  = _mulle_objc_classpair_get_infraclass( pair);
   found  = NULL;
   is_meta = _mulle_objc_class_is_metaclass( cls);

   rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   while( proto_cls = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      if( proto_cls == infra)
         continue;

      walk_cls = _mulle_objc_infraclass_as_class( proto_cls);
      if( is_meta)
         walk_cls = _mulle_objc_metaclass_as_class( _mulle_objc_infraclass_get_metaclass( proto_cls));

      method = _mulle_objc_class_search_method( walk_cls, methodid, previous, owner, inheritance | walk_cls->inheritance);
      if( method)
      {
         if( found)
         {
            errno = EEXIST;
            return( NULL);
         }

         if( ! _mulle_objc_methoddescriptor_is_hidden_override_fatal( &method->descriptor))
            return( method);

         found = method;
      }
   }
   _mulle_objc_protocolclassenumerator_done( &rover);

   if( ! found)
      errno = ENOENT;  // thread safe errno is potentially expensive

   return( found);
}


static unsigned int   _mulle_objc_class_count_protocolspreloadinstancemethods( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class                     *walk_cls;
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_infraclass                *proto_cls;
   struct _mulle_objc_infraclass                *infra;
   struct _mulle_objc_protocolclassenumerator   rover;
   unsigned int                                 preloads;
   int                                          is_meta;

   preloads = 0;

   pair    = _mulle_objc_class_get_classpair( cls);
   infra   = _mulle_objc_classpair_get_infraclass( pair);
   rover   = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   is_meta = _mulle_objc_class_is_metaclass( cls);

   while( proto_cls = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      if( proto_cls == infra)
         continue;

      walk_cls = _mulle_objc_infraclass_as_class( proto_cls);
      if( is_meta)
         walk_cls = _mulle_objc_metaclass_as_class( _mulle_objc_infraclass_get_metaclass( proto_cls));

      preloads += _mulle_objc_class_count_preloadmethods( walk_cls);
   }
   _mulle_objc_protocolclassenumerator_done( &rover);

   return( preloads);
}


# pragma mark - methods

unsigned int   _mulle_objc_class_count_preloadmethods( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *dad;
   unsigned int               preloads;

   preloads = cls->preloads;
   if( ! (cls->inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      dad = cls;
      while( dad = dad->superclass)
         preloads += _mulle_objc_class_count_preloadmethods( dad);
   }

   if( ! (cls->inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
      preloads += _mulle_objc_class_count_protocolspreloadinstancemethods( cls);

   return( preloads);
}

// 0: continue
typedef   int (*mulle_objc_walk_methods_callback)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *);

int   _mulle_objc_class_walk_methods( struct _mulle_objc_class *cls, unsigned int inheritance , mulle_objc_walk_methods_callback f, void *userinfo)
{
   int                                                rval;
   struct _mulle_objc_methodlist                      *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                       n;
   unsigned int                                       tmp;

   // todo: need to lock class

   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);

   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( rval = _mulle_objc_methodlist_walk( list, f, cls, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      tmp = MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS;
      if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES)
         tmp |= MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES;

      if( rval = _mulle_objc_class_protocol_walk_methods( cls, tmp, f, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( cls->superclass && cls->superclass != cls)
         return( _mulle_objc_class_walk_methods( cls->superclass, inheritance, f, userinfo));
   }

   return( 0);
}


static void   trace_method_found( struct _mulle_objc_runtime *runtime,
                                  struct _mulle_objc_class *cls,
                                  struct _mulle_objc_methodlist *list,
                                  struct _mulle_objc_method *method,
                                  struct mulle_concurrent_pointerarrayreverseenumerator *rover)
{
   char                      buf[ s_mulle_objc_sprintf_functionpointer_buffer + 32];
   mulle_objc_categoryid_t   categoryid;
   char                      *s;

   fprintf( stderr, "mulle_objc_runtime %p trace: found in %s ",
           runtime,
           _mulle_objc_class_get_classtypename( cls));

   // it's a category ?
   if( list->owner)
   {
      categoryid = (mulle_objc_categoryid_t) (uintptr_t) list->owner;
      s = _mulle_objc_runtime_search_debughashname( runtime, categoryid);
      if( ! s)
      {
         sprintf( buf, "%08x", categoryid);
         s = buf;
      }

      fprintf( stderr, "category %s( %s) implementation ",
              cls->name,
              s);
   }
   else
      fprintf( stderr, "\"%s\" implementation ",
              cls->name);
   mulle_objc_sprintf_functionpointer( buf, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method));

   fprintf( stderr, "%s for methodid %08x ( \"%s\")\"\n",
           buf,
           method->descriptor.methodid,
           method->descriptor.name);
}


//
// if previous is set, the search will be done for the method that previous
// has overriden
//
struct _mulle_objc_method   *_mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid,
                                                              struct _mulle_objc_method *previous,
                                                              void *owner,
                                                              unsigned int inheritance)
{
   struct _mulle_objc_runtime                              *runtime;
   struct _mulle_objc_method                               *found;
   struct _mulle_objc_method                               *method;
   struct _mulle_objc_methodlist                           *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;
   unsigned int                                            tmp;

   assert( mulle_objc_class_is_current_thread_registered( cls));

   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   runtime = _mulle_objc_class_get_runtime( cls);
   if( runtime->debug.trace.method_searches)
   {
      fprintf( stderr, "mulle_objc_runtime %p trace: search %s %s for methodid %08x \"%s\" (owner=%p, previous=%p)\n",
            runtime,
            _mulle_objc_class_get_classtypename( cls),
            cls->name,
            methodid,
            mulle_objc_string_for_methodid( methodid),
            owner,
            previous ? _mulle_objc_method_get_implementation( previous) : NULL);
   }

   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);
   found = NULL;

   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( owner != MULLE_OBJC_ANY_OWNER && list->owner != owner)
         continue;

      method = _mulle_objc_methodlist_search( list, methodid);

      if( method)
      {
         if( previous)
         {
            if( previous == method)
            {
               previous = NULL;
               continue;
            }
         }

         if( found)
         {
            errno = EEXIST;
            return( NULL);
         }

         if( ! _mulle_objc_methoddescriptor_is_hidden_override_fatal( &method->descriptor))
         {
            if( runtime->debug.trace.method_searches)
               trace_method_found( runtime, cls, list, method, &rover);

            mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
            return( method);
         }

         found = method;
      }
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      tmp = 0;
      if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES)
         tmp |= MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES;

      //
      // A protocol could well have a category of the same name and it would
      // match, which would be unexpected or would it ? Probably not.
      // Generally: MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES is not
      // enabled, so it is a non-issue.
      //
      method = _mulle_objc_class_protocol_search_method( cls, methodid, previous, owner, tmp);
      if( method)
      {
         if( found)
         {
            errno = EEXIST;
            return( NULL);
         }

         if( ! _mulle_objc_methoddescriptor_is_hidden_override_fatal( &method->descriptor))
            return( method);

         found = method;
      }
   }

   //
   // searching the superclass for owner seems wanted
   //
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( cls->superclass)
      {
         method = _mulle_objc_class_search_method( cls->superclass, methodid, previous, owner, cls->superclass->inheritance);
         if( method)
         {
            if( found)
            {
               errno = EEXIST;
               return( NULL);
            }

            if( ! _mulle_objc_methoddescriptor_is_hidden_override_fatal( &method->descriptor))
               return( method);

            found = method;
         }
         else
         {
            if( errno == EEXIST)
               found = NULL;
         }
      }
   }

   if( ! found)
      errno = ENOENT;  // thread safe errno is potentially expensive

   return( found);
}


struct _mulle_objc_method  *mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                                            mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *method;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   if( ! cls)
   {
      errno = EINVAL;
      return( NULL);
   }

   method = _mulle_objc_class_search_method( cls, methodid, NULL, MULLE_OBJC_ANY_OWNER, cls->inheritance);
   if( ! method && errno == EEXIST)
      _mulle_objc_runtime_raise_inconsistency_exception( cls->runtime, "class \"%s\" hidden method override of %llx", _mulle_objc_class_get_name( cls), methodid);

   return( method);
}


struct _mulle_objc_method  *mulle_objc_class_search_non_inherited_method( struct _mulle_objc_class *cls,
                                                                          mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *method;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   if( ! cls)
   {
      errno = EINVAL;
      return( NULL);
   }

   method = _mulle_objc_class_search_method( cls, methodid, NULL, MULLE_OBJC_ANY_OWNER, cls->inheritance);
   if( ! method && errno == EEXIST)
      _mulle_objc_runtime_raise_inconsistency_exception( cls->runtime, "class \"%s\" hidden method override of %llx", _mulle_objc_class_get_name( cls), methodid);

   return( method);
}


#pragma mark - debug supprt



struct bouncy_info
{
   void                          *userinfo;
   struct _mulle_objc_runtime    *runtime;
   void                          *parent;
   mulle_objc_walkcallback_t     callback;
   mulle_objc_walkcommand_t      rval;
};


static int   bouncy_method( struct _mulle_objc_method *method,
                            struct _mulle_objc_class *cls,
                            void *userinfo)
{
   struct bouncy_info   *info;

   info       = userinfo;
   info->rval = (info->callback)( info->runtime, method, mulle_objc_walkpointer_is_method, NULL, cls, info->userinfo);
   return( mulle_objc_walkcommand_is_stopper( info->rval));
}


// don't expose, because it's bit too weird

mulle_objc_walkcommand_t
   mulle_objc_class_walk( struct _mulle_objc_class   *cls,
                          enum mulle_objc_walkpointertype_t  type,
                          mulle_objc_walkcallback_t   callback,
                          void *parent,
                          void *userinfo)
{
   struct _mulle_objc_runtime   *runtime;
   mulle_objc_walkcommand_t     cmd;
   struct bouncy_info           info;

   runtime = _mulle_objc_class_get_runtime( cls);
   cmd     = (*callback)( runtime, cls, type, NULL, parent, userinfo);
   if( cmd != mulle_objc_walk_ok)
      return( cmd);

   info.callback = callback;
   info.parent   = parent;
   info.userinfo = userinfo;
   info.runtime  = runtime;

   cmd = _mulle_objc_class_walk_methods( cls, _mulle_objc_class_get_inheritance( cls), bouncy_method, &info);
   return( cmd);
}

