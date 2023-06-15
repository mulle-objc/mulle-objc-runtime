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
#include "mulle-objc-methodcache.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"



# pragma mark - cache


static mulle_objc_cache_uint_t
   _class_search_minmethodcachesize( struct _mulle_objc_class *cls)
{
   mulle_objc_cache_uint_t   preloads;

   // these are definitely in the cache
   preloads = _mulle_objc_class_count_preloadmethods( cls) +
              _mulle_objc_universe_get_numberofpreloadmethods( cls->universe);
   return( preloads);
}


// this runs when the class is locked,
static void   _mulle_objc_class_setup_initial_cache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe      *universe;
   struct _mulle_objc_methodcache   *mcache;
   struct mulle_allocator           *allocator;
   mulle_objc_cache_uint_t          n_entries;
   void                             *found;

   // now setup the cache and let it rip, except when we don't ever want one
   universe  = _mulle_objc_class_get_universe( cls);

   // your chance to change the cache algorithm and initital size
   n_entries = _class_search_minmethodcachesize( cls);
   if( universe->callbacks.will_init_cache)
      n_entries = (*universe->callbacks.will_init_cache)( universe, cls, n_entries);

   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
   {
      allocator = _mulle_objc_universe_get_allocator( universe);
      mcache    = mulle_objc_methodcache_new( n_entries, allocator);

      assert( mcache);

      // trace this before the switch
      if( universe->debug.trace.method_cache)
         mulle_objc_universe_trace( universe, "new initial cache %p "
                                    "on %s %08x \"%s\" (%p) with %u entries",
                                    mcache,
                                    _mulle_objc_class_get_classtypename( cls),
                                    _mulle_objc_class_get_classid( cls),
                                    _mulle_objc_class_get_name( cls),
                                    cls,
                                    mcache->cache.size);
      //
      // count #caches. As long as there are zero true caches, the universe can
      // be much faster adding methods.
      //
      _mulle_atomic_pointer_increment( &cls->universe->cachecount_1);
   }
   else
   {
      mcache = &universe->empty_methodcache;
      if( universe->debug.trace.method_cache)
         mulle_objc_universe_trace( universe, "use \"always empty cache\" on "
                                    "%s %08x \"%s\" (%p)",
                                    _mulle_objc_class_get_classtypename( cls),
                                    _mulle_objc_class_get_classid( cls),
                                    _mulle_objc_class_get_name( cls),
                                    cls);
      // empty cache doesn't count towards "problem" caches
   }
   found = __mulle_atomic_pointer_cas( &cls->cachepivot.pivot.entries,
                                       mcache->cache.entries,
                                       universe->initial_methodcache.cache.entries);
   assert( found == universe->initial_methodcache.cache.entries);

   ((void)(found)); // use :-/ for compiler
}


static void
   _mulle_objc_class_setup_initial_cache_if_needed( struct _mulle_objc_class *cls)
{
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
      return;

   _mulle_objc_class_setup_initial_cache( cls);

   // this marks the class as ready to run
   _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY);
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
   _mulle_objc_infraclass_call_initialize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_method       *initialize;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_metaclass    *meta;
   mulle_objc_implementation_t     imp;
   char                            *name;
   int                             preserve;

   preserve = errno;

   universe = _mulle_objc_infraclass_get_universe( infra);
   meta     = _mulle_objc_infraclass_get_metaclass( infra);
   name     = _mulle_objc_infraclass_get_name( infra);

   // grab code from superclass
   // this is useful for MulleObjCSingleton
   initialize = mulle_objc_class_defaultsearch_method( &meta->base,
                                                       MULLE_OBJC_INITIALIZE_METHODID);
   if( ! initialize)
   {
      if( universe->debug.trace.initialize)
         mulle_objc_universe_trace( universe, "no +[%s initialize] found", name);

      errno = preserve;
      return;
   }

   if( universe->debug.trace.initialize)
      mulle_objc_universe_trace( universe,
                                 "call +[%s initialize]",
                                 _mulle_objc_metaclass_get_name( meta));

   imp   = _mulle_objc_method_get_implementation( initialize);
   if( universe->debug.trace.method_call)
      mulle_objc_class_trace_call( &meta->base,  // sic!
                                   infra,
                                   MULLE_OBJC_INITIALIZE_METHODID,
                                   NULL,
                                   imp);
   (*imp)( (struct _mulle_objc_object *) infra,
           MULLE_OBJC_INITIALIZE_METHODID,
           NULL);

   if( universe->debug.trace.initialize)
      mulle_objc_universe_trace( universe,
                                 "done +[%s initialize]",
                                 _mulle_objc_metaclass_get_name( meta));
   errno = preserve;
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
         _mulle_objc_infraclass_call_initialize( infra);

         _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE);

         // now we can let it rip
         _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_metaclass_as_class( meta));
         _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_infraclass_as_class( infra));
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




#pragma mark - empty_cache calls

// the empty cache is never filled with anything

static void   *_mulle_objc_object_call_slow( void *obj,
                                             mulle_objc_methodid_t methodid,
                                             void *parameter)
{
   struct _mulle_objc_class      *cls;
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;

   cls    = _mulle_objc_object_get_isa( obj);
   method = mulle_objc_class_search_method_nofail( cls, methodid);
   imp    = _mulle_objc_method_get_implementation( method);
   return( (*imp)( obj, methodid, parameter));
}


static void   *_mulle_objc_object_call_class_slow( void *obj,
                                                   mulle_objc_methodid_t methodid,
                                                   void *parameter,
                                                   struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;

   method = mulle_objc_class_search_method_nofail( cls, methodid);
   imp    = _mulle_objc_method_get_implementation( method);
   return( (*imp)( obj, methodid, parameter));
}


MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
struct _mulle_objc_method *
   _mulle_objc_class_superlookup_method_slow_nocache_nofail( struct _mulle_objc_class *cls,
                                                             mulle_objc_superid_t superid)
{
   struct _mulle_objc_method             *method;
   struct _mulle_objc_universe           *universe;
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_super              *p;

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   _mulle_objc_searcharguments_init_super( &args, p->methodid, p->classid);
   method = mulle_objc_class_search_method( cls,
                                            &args,
                                            cls->inheritance,
                                            NULL);
   if( ! method)
      method = _mulle_objc_class_get_forwardmethod_lazy_nofail( cls, args.args.methodid);
   return( method);
}


MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation_slow_nocache_nofail( struct _mulle_objc_class *cls,
                                                                     mulle_objc_superid_t superid)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;

   method = _mulle_objc_class_superlookup_method_slow_nocache_nofail( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   return( imp);
}


void  _mulle_objc_methodcache_init_empty_callbacks( struct _mulle_objc_methodcache *p)
{
   p->call         = _mulle_objc_object_call_class_slow;
   p->call2        = _mulle_objc_object_call_slow;
   p->superlookup  = _mulle_objc_class_superlookup_implementation_slow_nocache_nofail;
   p->superlookup2 = _mulle_objc_class_superlookup_implementation_slow_nocache_nofail;
}


#pragma mark - initial_methodcache callbacks

void   *_mulle_objc_object_call_class_needcache( void *obj,
                                                 mulle_objc_methodid_t methodid,
                                                 void *parameter,
                                                 struct _mulle_objc_class *cls)
{
   _mulle_objc_class_setup( cls);
   return( _mulle_objc_object_call_slow( obj, methodid, parameter));
}



void   *_mulle_objc_object_call_needcache( void *obj,
                                            mulle_objc_methodid_t methodid,
                                            void *parameter)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( (struct _mulle_objc_object *) obj);
   _mulle_objc_class_setup( cls);
   return( _mulle_objc_object_call_slow( obj, methodid, parameter));
}


mulle_objc_implementation_t
   _mulle_objc_class_superlookup_needcache( struct _mulle_objc_class *cls,
                                             mulle_objc_superid_t superid)
{
   // happens when we do +[super initialize] in +initialize
   _mulle_objc_class_setup( cls);
   // this is slow and uncached as we need it
   return( _mulle_objc_class_superlookup_implementation_nocache_nofail( cls, superid));
}



void  _mulle_objc_methodcache_init_initial_callbacks( struct _mulle_objc_methodcache *p)
{
   p->call         = _mulle_objc_object_call_class_needcache;
   p->call2        = _mulle_objc_object_call_needcache;
   p->superlookup  = _mulle_objc_class_superlookup_needcache;
   p->superlookup2 = _mulle_objc_class_superlookup_needcache;
}

