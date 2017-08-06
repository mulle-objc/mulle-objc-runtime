//
//  mulle_objc_class.c
//  mulle-objc-runtime
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
#include "mulle_objc_universe.h"
#include "mulle_objc_taggedpointer.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_concurrent/mulle_concurrent.h>


// public but not publizied


# pragma mark - accessor

static char   *lookup_bitname( unsigned int bit)
{
   // some "known" values
   switch( bit)
   {
   case MULLE_OBJC_CLASS_CACHE_READY        : return( "CACHE_READY");
   case MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE : return( "ALWAYS_EMPTY_CACHE");
   case MULLE_OBJC_CLASS_FIXED_SIZE_CACHE   : return( "FIXED_SIZE_CACHE");
   case _MULLE_OBJC_CLASS_WARN_PROTOCOL     : return( "WARN_PROTOCOL");
   case _MULLE_OBJC_CLASS_IS_PROTOCOLCLASS  : return( "IS_PROTOCOLCLASS");
   case _MULLE_OBJC_CLASS_LOAD_SCHEDULED    : return( "LOAD_SCHEDULED");
   case MULLE_OBJC_CLASS_INITIALIZE_DONE    : return( "INITIALIZE_DONE");
   }
   return( 0);
}


int   _mulle_objc_class_set_state_bit( struct _mulle_objc_class *cls,
                                       unsigned int bit)
{
   void   *state;
   void   *old;
   struct _mulle_objc_universe   *universe;
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

   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.state_bits)
   {
      bitname = lookup_bitname( bit);
      fprintf( stderr, "mulle_objc_universe %p trace: %s %08x \"%s\" "
                       "gained the 0x%x bit (%s)\n",
                       universe, _mulle_objc_class_get_classtypename( cls),
                       cls->classid, cls->name, bit, bitname ? bitname : "???");
   }
   return( 1);
}


# pragma mark - initialization / deallocation

void   *_mulle_objc_object_call_class_needs_cache( void *obj,
                                                   mulle_objc_methodid_t methodid,
                                                   void *parameter,
                                                   struct _mulle_objc_class *cls);


void   _mulle_objc_class_init( struct _mulle_objc_class *cls,
                               char *name,
                               size_t  instancesize,
                               mulle_objc_classid_t classid,
                               struct _mulle_objc_class *superclass,
                               struct _mulle_objc_universe *universe)
{
   extern void   *_mulle_objc_object_call2_needs_cache( void *obj,
                                                        mulle_objc_methodid_t methodid,
                                                        void *parameter);

   assert( universe);

   cls->name                     = name;

   cls->superclass               = superclass;
   cls->superclassid             = superclass ? superclass->classid : MULLE_OBJC_NO_CLASSID;
   //   cls->nextclass                = superclass;
   cls->classid                  = classid;
   cls->allocationsize           = sizeof( struct _mulle_objc_objectheader) + instancesize;
   cls->call                     = _mulle_objc_object_call_class_needs_cache;
   cls->universe                 = universe;
   cls->inheritance              = universe->classdefaults.inheritance;

   cls->cachepivot.call2         = _mulle_objc_object_call2_needs_cache;

   _mulle_atomic_pointer_nonatomic_write( &cls->searchcachepivot.entries, universe->empty_searchcache.entries);
   _mulle_atomic_pointer_nonatomic_write( &cls->cachepivot.pivot.entries, universe->empty_cache.entries);
   _mulle_atomic_pointer_nonatomic_write( &cls->kvc.entries, universe->empty_cache.entries);

   _mulle_concurrent_pointerarray_init( &cls->methodlists, 0, &universe->memory.allocator);

   _mulle_objc_fastmethodtable_init( &cls->vtab);
}


void   _mulle_objc_class_done( struct _mulle_objc_class *cls,
                               struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_searchcache   *searchcache;

   assert( cls);
   assert( allocator);

   _mulle_objc_fastmethodtable_done( &cls->vtab);

   _mulle_concurrent_pointerarray_done( &cls->methodlists);

   _mulle_objc_class_invalidate_all_kvcinfos( cls);

   cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   if( cache != &cls->universe->empty_cache)
      _mulle_objc_cache_free( cache, allocator);

   searchcache = _mulle_objc_searchcachepivot_atomic_get_cache( &cls->searchcachepivot);
   if( searchcache != &cls->universe->empty_searchcache)
      _mulle_objc_searchcache_free( searchcache, allocator);
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
   if( ! cls ||
       (cls->classid == MULLE_OBJC_NO_CLASSID) ||
       (cls->classid == MULLE_OBJC_INVALID_CLASSID))
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


# pragma mark - caches

static int
   _mulle_objc_class_invalidate_methodcache( struct _mulle_objc_class *cls,
                                             mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;
   struct _mulle_objc_cache        *cache;

   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return( 0);
   
   cache = _mulle_objc_class_get_methodcache( cls);
   if( ! _mulle_atomic_pointer_read( &cache->n))
      return( 0);

   offset = _mulle_objc_cache_find_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   // no entry is matching, fine
   if( ! entry->key.uniqueid)
      return( 0);

   //
   // if we get NULL, from _mulle_objc_class_add_entry_by_swapping_caches
   // someone else recreated the cache, fine by us!
   //
   for(;;)
   {
      // always break regardless of return value
      _mulle_objc_class_add_cacheentry_by_swapping_caches( cls, cache, NULL, MULLE_OBJC_NO_UNIQUEID);
      break;
   }

   return( 0x1);
}


static int
   _mulle_objc_class_invalidate_searchcache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_searchcache   *searchcache;
   
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return( 0);
   
   searchcache = _mulle_objc_class_get_searchcache( cls);
   if( ! _mulle_atomic_pointer_read( &searchcache->n))
      return( 0);
   
   //
   // if we get NULL, from _mulle_objc_class_add_entry_by_swapping_caches
   // someone else recreated the cache, fine by us!
   //
   for(;;)
   {
      // always break regardless of return value
      _mulle_objc_class_add_searchcacheentry_by_swapping_caches( cls, searchcache, NULL, NULL);
      break;
   }
   
   return( 0x1);
}



static int  invalidate_caches( struct _mulle_objc_universe *universe,
                               struct _mulle_objc_class *cls,
                               enum mulle_objc_walkpointertype_t type,
                               char *key,
                               void *parent,
                               struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_methodlistenumerator   rover;
   struct _mulle_objc_method                 *method;

   // preferably nothing there yet
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
      return( mulle_objc_walk_ok);

   _mulle_objc_class_invalidate_all_kvcinfos( cls);

   // searchcaches need to be invalidate regardless
   _mulle_objc_class_invalidate_searchcache( cls);
   
   // if caches have been cleaned for class, it's done
   rover = _mulle_objc_methodlist_enumerate( list);
   while( method = _mulle_objc_methodlistenumerator_next( &rover))
      if( _mulle_objc_class_invalidate_methodcache( cls, method->descriptor.methodid))
         break;
   _mulle_objc_methodlistenumerator_done( &rover);

   return( mulle_objc_walk_ok);
}


#pragma mark - cache handling

//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
struct _mulle_objc_searchcacheentry  empty_entry;

MULLE_C_NEVER_INLINE
struct _mulle_objc_searchcacheentry   *
   _mulle_objc_class_add_searchcacheentry_by_swapping_caches( struct _mulle_objc_class *cls,
                                                              struct _mulle_objc_searchcache *cache,
                                                              struct _mulle_objc_method *method,
                                                              struct _mulle_objc_searchargumentscachable *args)
{
   struct _mulle_objc_searchcache        *old_cache;
   struct _mulle_objc_searchcacheentry   *entry;
   struct _mulle_objc_universe           *universe;
   mulle_objc_cache_uint_t               new_size;
   
   old_cache = cache;
   
   // a new beginning.. let it be filled anew
   new_size = cache->size * 2;
   cache    = mulle_objc_searchcache_new( new_size, &cls->universe->memory.allocator);
   
   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;
   if( method)
      entry = _mulle_objc_searchcache_inactivecache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), args);

   universe = _mulle_objc_class_get_universe( cls);

   // if the set fails, then someone else was faster
   if( _mulle_objc_searchcachepivot_atomic_set_entries( &cls->searchcachepivot, cache->entries, old_cache->entries))
   {
      _mulle_objc_searchcache_free( cache, &universe->memory.allocator); // sic, can be unsafe deleted now
      return( NULL);
   }
   
   if( universe->debug.trace.method_caches)
      fprintf( stderr, "mulle_objc_universe %p trace: new search cache %p for %s %08x \"%s\" with %u entries\n",
              universe,
              cache,
              _mulle_objc_class_get_classtypename( cls),
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls),
              cache->size);
   
   if( &old_cache->entries[ 0] != &universe->empty_searchcache.entries[ 0])
   {
      if( universe->debug.trace.method_caches)
         fprintf( stderr, "mulle_objc_universe %p trace: free old search cache %p for %s %08x \"%s\" with %u entries\n",
                 universe,
                 cache,
                 _mulle_objc_class_get_classtypename( cls),
                 _mulle_objc_class_get_classid( cls),
                 _mulle_objc_class_get_name( cls),
                 cache->size);
      
      _mulle_objc_searchcache_abafree( old_cache, &cls->universe->memory.allocator);
   }
   return( entry);
}


#pragma mark - methods

//
// The assumption on the runtime is, that you can only add but don't
// interpose. In this regard caches might be outdated but not wrong.
// TODO: What happens when a class gets adds a methodlist and
// the a method expects the superclass to also have a methodlist added
// already, which it hasn't. Solved by +dependencies!
// A problem remains: the superclass gets an incompatible method added,
// overriding the old, but the class isn't updated yet and a call
// happens which then supercalls.
// Solution: late categories may only add methods, not overwrite
//
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
      if( _mulle_atomic_pointer_read( &cls->universe->cachecount_1))
         mulle_objc_universe_walk_classes( cls->universe, (int (*)()) invalidate_caches, list);
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
      list = &cls->universe->empty_methodlist;

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
      if( _mulle_objc_universe_add_methoddescriptor( cls->universe, &method->descriptor))
      {
         _mulle_objc_methodlistenumerator_done( &rover);
         // errno = EEXIST; // errno set by _add_...
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


void   mulle_objc_class_unfailingadd_methodlist( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_methodlist *list)
{
   if( mulle_objc_class_add_methodlist( cls, list))
      _mulle_objc_universe_raise_fail_errno_exception( cls->universe);
}



static int   _mulle_objc_class_protocol_walk_methods( struct _mulle_objc_class *cls,
                                                      unsigned int inheritance,
                                                      int (*f)( struct _mulle_objc_method *,
                                                                struct _mulle_objc_class *,
                                                                void *),
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
   int                                                     rval;
   struct _mulle_objc_methodlist                           *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;
   unsigned int                                            tmp;

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


#pragma mark - debug support

struct bouncy_info
{
   void                          *userinfo;
   struct _mulle_objc_universe   *universe;
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
   info->rval = (info->callback)( info->universe, method, mulle_objc_walkpointer_is_method, NULL, cls, info->userinfo);
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
   struct _mulle_objc_universe   *universe;
   mulle_objc_walkcommand_t      cmd;
   struct bouncy_info            info;

   universe = _mulle_objc_class_get_universe( cls);
   cmd     = (*callback)( universe, cls, type, NULL, parent, userinfo);
   if( cmd != mulle_objc_walk_ok)
      return( cmd);

   info.callback = callback;
   info.parent   = parent;
   info.userinfo = userinfo;
   info.universe  = universe;

   cmd = _mulle_objc_class_walk_methods( cls, _mulle_objc_class_get_inheritance( cls), bouncy_method, &info);
   return( cmd);
}

