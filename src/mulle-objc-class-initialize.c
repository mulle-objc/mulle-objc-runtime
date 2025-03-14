//
//  mulle-objc-class-initialize.c
//  mulle-objc-runtime
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
//  Copyright (c) 2021 Codeon GmbH.
//  All rights reserved.
//
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
#include "mulle-objc-class-initialize.h"

#include "include-private.h"

#include "mulle-objc-class.h"
#include "mulle-objc-class-struct.h"
#include "mulle-objc-class-lookup.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-call.h"
#include "mulle-objc-impcache.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"



# pragma mark - cache

//
// MEMO: we could traverse the universe super list of the universe and then
//       retrieve the classid and fill the cache for the desired methodid.
//       Then stepping through super calls with the debugger should be less
//       painful.
//
static mulle_objc_cache_uint_t
   _mulle_objc_class_search_min_impcache_size( struct _mulle_objc_class *cls)
{
   mulle_objc_cache_uint_t   preloads;

   // these are definitely in the cache
   preloads = _mulle_objc_class_count_preloadmethods( cls) +
              _mulle_objc_universe_get_numberofpreloadmethods( cls->universe);
   return( preloads);
}


// This is the **initialize** cache not the **initial** cache.
//
// This runs before +initialize run. Its there so the method cache won't be
// used during execution of +initialize
//
static void   _mulle_objc_class_setup_initialize_cache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_impcache   *icache;
   void                          *found;

   // now setup the cache and let it rip, except when we don't ever want one
   universe = _mulle_objc_class_get_universe( cls);
   icache   = &universe->empty_impcache;
   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe, "use \"always empty cache\" on for +initialize "
                                           "%s %08lx \"%s\" (%p)",
                                           _mulle_objc_class_get_classtypename( cls),
                                           (unsigned long) _mulle_objc_class_get_classid( cls),
                                           _mulle_objc_class_get_name( cls),
                                           cls);
   // empty cache doesn't count towards "problem" caches
   found = __mulle_atomic_pointer_cas( &cls->cachepivot.pivot.entries,
                                       icache->cache.entries,
                                       universe->initial_impcache.cache.entries);
   assert( found == universe->initial_impcache.cache.entries);

   MULLE_C_UNUSED( found); // use :-/ for compiler
}


//
// this runs when the class is locked
//
// the **initial** cache is not the **initialize** cache!
//
static void   _mulle_objc_class_setup_initial_cache( struct _mulle_objc_class *cls,
                                                     struct _mulle_objc_impcache_callback *callback)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_impcache   *icache;
   struct mulle_allocator        *allocator;
   mulle_objc_cache_uint_t       n_entries;
   void                          *found;

   // always empty cache is already installed
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return;

   // your chance to change the cache algorithm and initial size
   universe  = _mulle_objc_class_get_universe( cls);
   n_entries = _mulle_objc_class_search_min_impcache_size( cls) * 3 / 2;  // leave some holes
   if( universe->callbacks.will_init_cache)
      n_entries = (*universe->callbacks.will_init_cache)( universe, cls, n_entries);

   allocator = _mulle_objc_universe_get_allocator( universe);
   icache    = mulle_objc_impcache_new( n_entries, callback, allocator);

   assert( icache);

   // trace this before the switch
   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe, "new initial cache %p "
                                 "on %s %08lx \"%s\" (%p) with %u entries",
                                 icache,
                                 _mulle_objc_class_get_classtypename( cls),
                                 (unsigned long) _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls),
                                 cls,
                                 icache->cache.size);
   //
   // count #caches. As long as there are zero true caches, the universe can
   // be much faster adding methods.
   //
   _mulle_atomic_pointer_increment( &cls->universe->cachecount_1);

   //
   // if someone else setup the cache during +initialize, that's OK, but
   // he must have set MULLE_OBJC_CLASS_CACHE_READY to ON so this
   // _mulle_objc_class_setup_initial_cache won't even be called
   //
   found = __mulle_atomic_pointer_cas( &cls->cachepivot.pivot.entries,
                                       icache->cache.entries,
                                       universe->empty_impcache.cache.entries);
   assert( found == universe->empty_impcache.cache.entries);

   MULLE_C_UNUSED( found); // use :-/ for compiler
}


int   _mulle_objc_class_setup_initial_cache_if_needed( struct _mulle_objc_class *cls,
                                                       struct _mulle_objc_impcache_callback *callback)
{
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
      return( 0);

   _mulle_objc_class_setup_initial_cache( cls, callback);

   // this marks the class as ready to run
   _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY);
   return( 1);
}


# pragma mark - +initialize


MULLE_C_NEVER_INLINE
void   _mulle_objc_class_warn_recursive_initialize( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.initialize)
   {
      mulle_objc_universe_trace( universe, "recursive +[%s initialize] ignored.\n"
                     "break on _mulle_objc_class_warn_recursive_initialize to debug.",
              _mulle_objc_class_get_name( cls));
      mulle_objc_universe_maybe_hang_or_abort( universe);
   }
}


static void
   _mulle_objc_metaclass_call_initialize_deinitialize( struct _mulle_objc_metaclass *meta,
                                                       unsigned int inheritance,
                                                       mulle_objc_methodid_t methodid,
                                                       struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_method            *method;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   mulle_objc_implementation_t          imp;
   int                                  preserve;
   int                                  is_initialize;

   preserve = errno;

   search = mulle_objc_searcharguments_make_default( methodid);
   method = mulle_objc_class_search_method( &meta->base,
                                            &search,
                                            inheritance,
                                            &result);

   universe = _mulle_objc_metaclass_get_universe( meta);

   if( ! method)
   {
      if( universe->debug.trace.initialize)
      {
         is_initialize = (methodid == MULLE_OBJC_INITIALIZE_METHODID);
         mulle_objc_universe_trace( universe, "no +[%s %sinitialize] found",
                                               _mulle_objc_metaclass_get_name( meta),
                                               is_initialize ? "" : "de");
      }
      return;
   }

   if( universe->debug.trace.initialize)
      mulle_objc_universe_trace( universe,
                                 "call +[%s %s] for class \"%s\"",
                                 _mulle_objc_class_get_name( result.class),
                                 _mulle_objc_method_get_name( method),
                                 _mulle_objc_infraclass_get_name( infra));

   imp = _mulle_objc_method_get_implementation( method);
//   if( universe->debug.method_call)
//      _mulle_objc_implementation_debug( imp,
//                                        infra,
//                                        _mulle_objc_method_get_methodid( initialize),
//                                        NULL,
//                                        &meta->base); // sic /

   mulle_objc_implementation_invoke( imp,
                                    (struct _mulle_objc_object *) infra,
                                    _mulle_objc_method_get_methodid( method),
                                    (struct _mulle_objc_object *) infra);

   if( universe->debug.trace.initialize)
      mulle_objc_universe_trace( universe,
                                 "done +[%s %s] for class \"%s\"",
                                 _mulle_objc_class_get_name( result.class),
                                 _mulle_objc_method_get_name( method),
                                 _mulle_objc_infraclass_get_name( infra));

   errno = preserve;
}


static void
   _mulle_objc_infraclass_call_protocolclasses_initialize( struct _mulle_objc_infraclass *owner,
                                                           struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_protocolclassenumerator   rover;
   struct _mulle_objc_infraclass                *superclass;
   struct _mulle_objc_infraclass                *protocolclass;
   struct _mulle_objc_metaclass                 *metaprotocolclass;

   assert( ! _mulle_objc_infraclass_get_state_bit( owner, MULLE_OBJC_CLASS_IS_PROTOCOLCLASS));

   // call all +initialize protocolclass methods for superclass first
   superclass = _mulle_objc_infraclass_get_superclass( owner);
   if( superclass)
      _mulle_objc_infraclass_call_protocolclasses_initialize( superclass, infra);

   pair  = _mulle_objc_infraclass_get_classpair( owner);
   rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   while( protocolclass = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      metaprotocolclass  = _mulle_objc_infraclass_get_metaclass( protocolclass);
      // just inherit from class nothing else
      _mulle_objc_metaclass_call_initialize_deinitialize( metaprotocolclass,
                                                          ~MULLE_OBJC_CLASS_DONT_INHERIT_CLASS,
                                                          MULLE_OBJC_INITIALIZE_METHODID,
                                                          infra);
   }
   _mulle_objc_protocolclassenumerator_done( &rover);
}



static void
   _mulle_objc_infraclass_call_initialize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass   *meta;
   struct _mulle_objc_class       *cls;
   unsigned int                   inheritance;

   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_CLASS_IS_PROTOCOLCLASS))
      return;

   meta         = _mulle_objc_infraclass_get_metaclass( infra);
   cls          = _mulle_objc_metaclass_as_class( meta);
   inheritance  = _mulle_objc_class_get_inheritance( cls);
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS;
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES;
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META;
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS_INHERITANCE;

   _mulle_objc_metaclass_call_initialize_deinitialize( meta,
                                                       inheritance,
                                                       MULLE_OBJC_INITIALIZE_METHODID,
                                                       infra);
   //
   _mulle_objc_infraclass_call_protocolclasses_initialize( infra, infra);
}


static void
   _mulle_objc_infraclass_call_protocolclasses_deinitialize( struct _mulle_objc_infraclass *owner,
                                                             struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair                        *pair;
   struct _mulle_objc_protocolclassreverseenumerator   rover;
   struct _mulle_objc_infraclass                       *superclass;
   struct _mulle_objc_infraclass                       *protocolclass;
   struct _mulle_objc_metaclass                        *metaprotocolclass;

   assert( ! _mulle_objc_infraclass_get_state_bit( owner, MULLE_OBJC_CLASS_IS_PROTOCOLCLASS));

   pair  = _mulle_objc_infraclass_get_classpair( owner);
   rover = _mulle_objc_classpair_reverseenumerate_protocolclasses( pair);
   while( protocolclass = _mulle_objc_protocolclassreverseenumerator_next( &rover))
   {
      metaprotocolclass = _mulle_objc_infraclass_get_metaclass( protocolclass);
      // just inherit from class nothing else
      _mulle_objc_metaclass_call_initialize_deinitialize( metaprotocolclass,
                                                          ~MULLE_OBJC_CLASS_DONT_INHERIT_CLASS,
                                                          MULLE_OBJC_DEINITIALIZE_METHODID,
                                                          infra);
   }
   _mulle_objc_protocolclassreverseenumerator_done( &rover);

   // call all +initialize protocolclass methods for superclass first
   superclass = _mulle_objc_infraclass_get_superclass( owner);
   if( superclass)
      _mulle_objc_infraclass_call_protocolclasses_deinitialize( superclass, infra);

}


void
   _mulle_objc_infraclass_call_deinitialize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass   *meta;
   struct _mulle_objc_class       *cls;
   unsigned int                   inheritance;

   // do not deinitialize which has never been initialized
   if( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
      return;

   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_CLASS_IS_PROTOCOLCLASS))
      return;

   _mulle_objc_infraclass_call_protocolclasses_deinitialize( infra, infra);

   meta         = _mulle_objc_infraclass_get_metaclass( infra);
   cls          = _mulle_objc_metaclass_as_class( meta);
   inheritance  = _mulle_objc_class_get_inheritance( cls);
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS;
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES;
//   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META;

   _mulle_objc_metaclass_call_initialize_deinitialize( meta, inheritance, MULLE_OBJC_DEINITIALIZE_METHODID, infra);
}



static void  _mulle_objc_infraclass_setup_superclasses( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_protocolclassenumerator   rover;
   struct _mulle_objc_infraclass                *protocolclass;
   struct _mulle_objc_infraclass                *superclass;

   /*
    * Ensure protocol classes are there
    */
   pair  = _mulle_objc_infraclass_get_classpair( infra);
   rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   while( protocolclass = _mulle_objc_protocolclassenumerator_next( &rover))
      _mulle_objc_infraclass_setup_if_needed( protocolclass);
   _mulle_objc_protocolclassenumerator_done( &rover);

   /*
    * Ensure superclass is there
    */
   superclass = _mulle_objc_infraclass_get_superclass( infra);
   if( superclass)
      _mulle_objc_infraclass_setup_if_needed( superclass);
}


static void  _mulle_objc_metaclass_setup_superclass( struct _mulle_objc_metaclass *meta)
{
   struct _mulle_objc_class        *superclass;
   struct _mulle_objc_infraclass   *infra;

   assert( _mulle_objc_class_is_metaclass( _mulle_objc_metaclass_as_class( meta)));

   /*
    * Ensure superclass is there (infraclass will do protocolclasses)
    */
   superclass = _mulle_objc_metaclass_get_superclass( meta);
   if( superclass)
   {
      // this is a little convoluted, because the root superclass could
      // actually be an infraclass (NSObject)
      infra = _mulle_objc_class_get_infraclass( superclass);
      if( ! infra) // can happen if we hit root
         infra = _mulle_objc_class_as_infraclass( superclass);

      _mulle_objc_infraclass_setup_if_needed( infra);
   }
}


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_impcache_callback  _mulle_objc_impcache_callback_empty;


int   _mulle_objc_class_setup( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_classpair    *pair;
   mulle_thread_mutex_t            *initialize_lock;
   mulle_thread_t                  current_thread;

   assert( mulle_objc_class_is_current_thread_registered( cls));

   pair           = _mulle_objc_class_get_classpair( cls);
   infra          = _mulle_objc_classpair_get_infraclass( pair);
   current_thread = mulle_thread_self();

   //
   // Allow recursion to same class in same thread
   // if a second thread is incoming we want to lock, so that the other
   // thread can finish up.
   // With the new code, we shouldn't really be getting here anymore
   //
   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZING))
   {
      // if we are the same thread, we do the slow call here
      if( (mulle_thread_t) _mulle_atomic_pointer_read( &pair->thread) == current_thread)
      {
         // is this still true ?
         if( cls->superclass)
            _mulle_objc_class_warn_recursive_initialize( cls);  // hmmm
         return( 1);  // go slow
      }
   }

   initialize_lock = _mulle_objc_classpair_get_lock( pair);
   mulle_thread_mutex_lock( initialize_lock);
   {
      // this has to be set before initializing
      // we make this atomic, just to be sure that it's set before
      // initializing...
      _mulle_atomic_pointer_write( &pair->thread, (void *) current_thread);
      if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZING))
      {
         // #>>> PROBLEMATIC #

         // As soon as we setup a method cache, the class is free to be
         // messaged by other threads. This means we have to run +initialize
         // without a cache. Other threads must be blocked in the
         // initialize_lock.
         //
         meta = _mulle_objc_classpair_get_metaclass( pair);
         _mulle_objc_metaclass_setup_superclass( meta);
         _mulle_objc_infraclass_setup_superclasses( infra);


         // MEMO: we are in state MULLE_OBJC_INFRACLASS_INITIALIZING, but
         //       not yet MULLE_OBJC_INFRACLASS_INITIALIZE_DONE. The
         //       superclasses are setup already though. And they ran
         //       (or are running (!)) +initialize.
         //       It is only guaranteed that +initialize on the superclass is
         //       messaged before the subclass, it isn't guaranteed that it has
         //       completed.

         _mulle_objc_class_setup_initialize_cache( _mulle_objc_metaclass_as_class( meta));
         _mulle_objc_class_setup_initialize_cache( _mulle_objc_infraclass_as_class( infra));

         _mulle_objc_infraclass_call_initialize( infra);

         //
         // now we can let it rip, NULL is standard callbacks
         // We can not do _mulle_objc_infraclass_call_initialize before
         // because we then get into recursion problems setting up the
         // cache. If we want to change the cache callbacks, we do what ?
         //
         _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_metaclass_as_class( meta), NULL);
         _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_infraclass_as_class( infra), NULL);


         _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE);
      }
      else
      {
         // looks like someone else was quicker, fine
         assert( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE));
      }
   }
   mulle_thread_mutex_unlock( initialize_lock);
   return( 0);
}


#pragma mark - empty_cache callbacks

// the empty cache is never filled with anything


static void   *_mulle_objc_object_call_class_slow( void *obj,
                                                   mulle_objc_methodid_t methodid,
                                                   void *parameter,
                                                   struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;
   void                          *result;

   method = mulle_objc_class_search_method_nofail( cls, methodid);
   imp    = _mulle_objc_method_get_implementation( method);
   result = mulle_objc_implementation_invoke( imp, obj, methodid, parameter);
   return( result);
}


static void   *_mulle_objc_object_call2_slow( void *obj,
                                             mulle_objc_methodid_t methodid,
                                             void *parameter)
{
   struct _mulle_objc_class      *cls;
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;
   void                          *result;

   cls    = _mulle_objc_object_get_isa( obj);
   method = mulle_objc_class_search_method_nofail( cls, methodid);
   imp    = _mulle_objc_method_get_implementation( method);
   result = mulle_objc_implementation_invoke( imp, obj, methodid, parameter);
   return( result);
}


static void *
   _mulle_objc_object_call_super_slow( void *obj,
                                      mulle_objc_methodid_t methodid,
                                      void *parameter,
                                      mulle_objc_superid_t superid,
                                      struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;
   void                          *result;

   MULLE_C_UNUSED( obj);

   method = _mulle_objc_class_supersearch_method_nofail( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   result = mulle_objc_implementation_invoke( imp, obj, methodid, parameter);
   return( result);
}


struct _mulle_objc_impcache_callback  _mulle_objc_impcache_callback_empty =
{
   .call                       = _mulle_objc_object_call_class_slow,
   .call_cache_collision       = _mulle_objc_object_call2_slow,
   .call_cache_miss            = _mulle_objc_object_call2_slow,
   .refresh_method_nofail      = _mulle_objc_class_refresh_method_nofail,
   .supercall                  = _mulle_objc_object_call_super_slow,
   .supercall_cache_collision  = _mulle_objc_object_call_super_slow,
   .supercall_cache_miss       = _mulle_objc_object_call_super_slow,
   .refresh_supermethod_nofail = _mulle_objc_class_refresh_supermethod_nofail
};


#pragma mark - initial_impcache callbacks

static void   *_mulle_objc_object_call_class_needcache( void *obj,
                                                        mulle_objc_methodid_t methodid,
                                                        void *parameter,
                                                        struct _mulle_objc_class *cls)
{
   void   *result;

   _mulle_objc_class_setup( cls);
   result = _mulle_objc_object_call_class_slow( obj, methodid, parameter, cls);
   return( result);
}



static void   *_mulle_objc_object_call2_needcache( void *obj,
                                                   mulle_objc_methodid_t methodid,
                                                   void *parameter)
{
   struct _mulle_objc_class   *cls;
   void                       *result;

   cls    = _mulle_objc_object_get_isa( (struct _mulle_objc_object *) obj);
   result = _mulle_objc_object_call_class_needcache( obj, methodid, parameter, cls);
   return( result);
}


static void *
   _mulle_objc_object_call_super_needcache( void *obj,
                                            mulle_objc_methodid_t methodid,
                                            void *parameter,
                                            mulle_objc_superid_t superid,
                                            struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_method     *method;
   void                          *result;

   // happens when we do +[super initialize] in +initialize
   _mulle_objc_class_setup( cls);

   // this is slow and uncached as we need it
   method = _mulle_objc_class_search_supermethod_nofail( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   result = mulle_objc_implementation_invoke( imp, obj, methodid, parameter);
   return( result);
}



struct _mulle_objc_impcache_callback   _mulle_objc_impcache_callback_initial =
{
   .call                       = _mulle_objc_object_call_class_needcache,
   .call_cache_collision       = _mulle_objc_object_call2_needcache,
   .call_cache_miss            = _mulle_objc_object_call2_needcache,
   .supercall                  = _mulle_objc_object_call_super_needcache,
   .supercall_cache_collision  = _mulle_objc_object_call_super_needcache,
   .supercall_cache_miss       = _mulle_objc_object_call_super_needcache,

   .refresh_method_nofail      = _mulle_objc_class_refresh_method_nofail,
   .refresh_supermethod_nofail = _mulle_objc_class_refresh_supermethod_nofail
};

