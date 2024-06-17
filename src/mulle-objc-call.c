//
//  mulle_objc_message.c
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
#include "mulle-objc-call.h"

#include "mulle-objc-class.h"
#include "mulle-objc-class-lookup.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-universe-class.h"
#include "mulle-objc-impcache.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-object.h"
#include "mulle-objc-retain-release.h"
#include "mulle-objc-universe.h"

#include "include-private.h"

#include <assert.h>
#include <stdlib.h>


//
// MEMO: these functions need to go through the memory cache vectors
//

#pragma mark - non-inline API calls


// we can't do the FCS code here, because its just super slow if
// the constant isn't known.
// This is like call_cache_collision, except that we still need do check the
// first cache entry.
MULLE_C_NEVER_INLINE
void   *mulle_objc_object_call( void *obj,
                                mulle_objc_methodid_t methodid,
                                void *parameter)
{
   return( mulle_objc_object_call_inline_full( obj, methodid, parameter));
}


// as above but no obj check
MULLE_C_NEVER_INLINE
void   *_mulle_objc_object_call( void *obj,
                                 mulle_objc_methodid_t methodid,
                                 void *parameter)
{
   return( _mulle_objc_object_call_inline_full( obj, methodid, parameter));
}



MULLE_C_NEVER_INLINE
void   *mulle_objc_object_call_super( void *obj,
                                     mulle_objc_methodid_t methodid,
                                     void *parameter,
                                     mulle_objc_superid_t superid)
{
   return( mulle_objc_object_call_super_inline_full( obj, methodid, parameter, superid));
}




# pragma mark - trace method

void   mulle_objc_implementation_trace( mulle_objc_implementation_t imp,
                                        void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        struct _mulle_objc_class *cls)
{
   struct _mulle_objc_descriptor        *desc;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_class             *isa;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   struct _mulle_objc_method            *method;
   unsigned int                         inheritance;
   char                                 *name;
   int                                  frames;

   universe = _mulle_objc_class_get_universe( cls);
   //
   // What is basically wrong here is, that we should be searching for the
   // class that implements the method, not the called class
   // This is fairly expensive though...
   //

   // we need the result for display, so need to re-search method
   inheritance = _mulle_objc_class_get_inheritance( cls);
   _mulle_objc_searcharguments_init_imp( &search, imp);
   method = mulle_objc_class_search_method( cls,
                                            &search,
                                            inheritance,
                                            &result);

   mulle_thread_mutex_lock( &universe->debug.lock);
   {
      mulle_objc_universe_trace_preamble( universe);

      fprintf( stderr, "[::] ");
      frames = (*universe->debug.count_stackdepth)();
      while( frames--)
         fputc( ' ', stderr);
      fprintf( stderr, "%c[", _mulle_objc_class_is_metaclass( cls) ? '+' : '-');

      if( method)
      {
         fprintf( stderr, "%s", _mulle_objc_class_get_name( result.class));
         name = mulle_objc_methodlist_get_categoryname( result.list);
         if( name)
            fprintf( stderr, "(%s)", name);
         fprintf( stderr, " %s]", _mulle_objc_method_get_name( method));
      }
      else
      {
         // fallback in case...
         fprintf( stderr, "?%s", _mulle_objc_class_get_name( cls));
         desc     = _mulle_objc_universe_lookup_descriptor( universe, methodid);
         if( desc)
            fprintf( stderr, " %s]", desc->name);
         else
            fprintf( stderr, " #%08lx]", (unsigned long) methodid);
      }

      isa = _mulle_objc_object_get_isa_universe( obj, universe);
      fprintf( stderr, " @%p %s (%p, #%08lx, %p)\n",
               imp,
               _mulle_objc_class_get_name( isa),
               obj,
               (unsigned long) methodid,
               parameter);
   }
   mulle_thread_mutex_unlock( &universe->debug.lock);
}



// MEMO: TAO and the Cache
//
// When we do TAO, we need to call this often. The idea is that all methods
// that are "threadsafe" can be moved into cache. Not threadsafe methods
// are kept out of the cache, therefore "refail" and will get checked again.
// Once TAO is disabled, these will be back in the cache and everything is
// proceeding as foreseen.
//
// "Direct" method invocations suffer though.
//
void   mulle_objc_object_taocheck_call( void *obj,
                                        mulle_objc_methodid_t methodid)
{
   mulle_thread_t                  object_thread;
   mulle_thread_t                  curr_thread;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_descriptor   *desc;

   //
   // these are no problem, as they are known to be threadsafe and
   // we want to get rid of them ASAP as they come in often
   // forward: is is really threadsafe ?
   //
   if( methodid == MULLE_OBJC_RELEASE_METHODID
       || methodid == MULLE_OBJC_RETAIN_METHODID
       || methodid == MULLE_OBJC_AUTORELEASE_METHODID)
   {
      return;
   }
   
   //
   // special case -finalize is not really threadsafe, but if the retainCount
   // is 0, we conclude only one thread still has access to the object and
   // we let it pass (we are in the simple -finalize/-dealloc). Unfortunately
   // when accessors are used to clear properties, this check won't
   // catch this. So we generalize to just do this check for all methods,
   // the only method scopes, where this should actually be true are -finalize
   // (if called before dealloc) and -dealloc.
   if( // methodid == MULLE_OBJC_FINALIZE_METHODID &&
       _mulle_objc_object_get_retaincount( obj) == 0)
   {
      return;
   }

   object_thread = _mulle_objc_object_get_thread( obj);
   if( ! object_thread)
      return;

   curr_thread = mulle_thread_self();

   //
   // dealloc is single threaded by default, change affinity to current thread
   // this way objects can dealloc in a worker thread (though how did they
   // get there ?)
   //
   // if( sel == MULLE_OBJC_DEALLOC_METHODID)
   // {
   //    _mulle_objc_object_set_thread( obj, curr_thread);
   //    return( 0);
   // }

   if( object_thread == curr_thread)
      return;

   //
   // check if selector references an -init or -dealloc method as these
   // are always thread safe (unless seriously misused)
   //
   cls    = _mulle_objc_object_get_isa( obj);
   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)  // must be forward, so check later
      return;

   // if threadsafe bit is set, then we are fiiiine
   // this probably shouldn't be in here, but some foundation callback
   if( ! _mulle_objc_method_is_threadaffine( method))
      return;

   desc = _mulle_objc_method_get_descriptor( method);
   switch( _mulle_objc_descriptor_get_methodfamily( desc))
   {
   case _mulle_objc_methodfamily_init    :
   case _mulle_objc_methodfamily_dealloc :
      return;
   default :
      break;
   }

   // forward: is not "methodid", that's what we forward to
   if( method->descriptor.methodid == MULLE_OBJC_FORWARD_METHODID)
      return;

   // OK so its wrong, but are we still multi-threaded ? could be that
   // the universe is winding down and releasing stuff
   // could ask [NSThread isMultithreaded], but it seems wrong
   universe = _mulle_objc_object_get_universe( obj);
   if( _mulle_objc_universe_is_deinitializing( universe))
      return;

   mulle_objc_universe_fail_wrongthread( universe, obj, object_thread, desc);
}


# pragma mark - normal callbacks for memorycache


//
// MEMO: these callbacks obviously don't go through the memory cache vectors
//       again. Also here is tracing and toachecking implemented
//
static void   *_mulle_objc_object_callback_class( void *obj,
                                                  mulle_objc_methodid_t methodid,
                                                  void *parameter,
                                                  struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_impcache      *icache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_method        *method;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( obj);
   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];

      if( entry->key.uniqueid == methodid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
         // direct cache hit, no trace(!), no tao check needed
         break;
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         method = (*icache->callback.refresh_method_nofail)( cls, methodid);
         imp    = _mulle_objc_method_get_implementation( method);
         imp    = _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
         break;
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }

   /*->*/
   // do not use invoke in method calls
   return( (*imp)( obj, methodid, parameter));
}


//
// this function is called, when the first inline cache check gave a
// collision, it skips the first found entry. This method is put into
// the method cache, you don't call it directly.
//
static void   *_mulle_objc_object_callback_cache_collision( void *obj,
                                                            mulle_objc_methodid_t methodid,
                                                            void *parameter)
{
   mulle_objc_implementation_t     imp;
   mulle_functionpointer_t         p;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache      *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls      = _mulle_objc_object_get_isa( obj);
   entries  = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache    = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask     = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp     = (mulle_objc_implementation_t) p;
         // no debug, since we don't put methods in the cache if we trace
         break;
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         method = (*icache->callback.refresh_method_nofail)( cls, methodid);
         imp    = _mulle_objc_method_get_implementation( method);
         imp    = _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
         break;
      }
   }

   /*->*/
   return( (*imp)( obj, methodid, parameter));
}


//
// this function is called, when there is no entry in the cache
//
static void   *_mulle_objc_object_callback_cache_miss( void *obj,
                                                       mulle_objc_methodid_t methodid,
                                                       void *parameter)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   icache  = _mulle_objc_cache_get_impcache_from_cache( cache);
   method  = (*icache->callback.refresh_method_nofail)( cls, methodid);
   imp     = _mulle_objc_method_get_implementation( method);
   imp     = _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
   return( (*imp)( obj, methodid, parameter));
}


//
// This function is called, when the first inline cache check gave a
// collision, it skips the first found entry. This is not called directly
// but placed into the method cache.
//
static void *
   _mulle_objc_object_callback_super( void *obj,
                                     mulle_objc_methodid_t methodid,
                                     void *parameter,
                                     mulle_objc_superid_t superid,
                                     struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_method       *method;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) superid;
   for(;;)
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == superid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
         break;
      }
      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         method = (*icache->callback.refresh_supermethod_nofail)( cls, superid);
         imp    = _mulle_objc_method_get_implementation( method);
         imp    = _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
         break;
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
/*->*/
   return( (*imp)( obj, methodid, parameter));
}


static void *
   _mulle_objc_object_callback_super_cache_collision( void *obj,
                                                      mulle_objc_methodid_t methodid,
                                                      void *parameter,
                                                      mulle_objc_superid_t superid,
                                                      struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_method       *method;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) superid;
   for(;;)
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == superid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
         break;
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         method = (*icache->callback.refresh_supermethod_nofail)( cls, superid);
         imp    = _mulle_objc_method_get_implementation( method);
         imp    = _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
         break;
      }
   }
   return( (*imp)( obj, methodid, parameter));
}


static void *
   _mulle_objc_object_callback_super_cache_miss( void *obj,
                                                 mulle_objc_methodid_t methodid,
                                                 void *parameter,
                                                 mulle_objc_superid_t superid,
                                                 struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   icache  = _mulle_objc_cache_get_impcache_from_cache( cache);
   method  = (*icache->callback.refresh_supermethod_nofail)( cls, superid);
   imp     = _mulle_objc_method_get_implementation( method);
   imp     = _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
   return( (*imp)( obj, methodid, parameter));
}


struct _mulle_objc_impcache_callback   _mulle_objc_impcache_callback_normal =
{
   .call                       = _mulle_objc_object_callback_class,
   .call_cache_collision       = _mulle_objc_object_callback_cache_collision,
   .call_cache_miss            = _mulle_objc_object_callback_cache_miss,
   .supercall                  = _mulle_objc_object_callback_super,  // public actually
   .supercall_cache_collision  = _mulle_objc_object_callback_super_cache_collision,
   .supercall_cache_miss       = _mulle_objc_object_callback_super_cache_miss,

   .refresh_method_nofail      = _mulle_objc_class_refresh_method_nofail,
   .refresh_supermethod_nofail = _mulle_objc_class_refresh_supermethod_nofail
};


// need to call cls->call to prepare caches

#pragma mark - multiple objects call

void   mulle_objc_objects_call( void **objects,
                                unsigned int n,
                                mulle_objc_methodid_t methodid,
                                void *params)
{
   mulle_objc_implementation_t   (*lookup)( struct _mulle_objc_class *, mulle_objc_methodid_t);
   mulle_objc_implementation_t   imp;
   mulle_objc_implementation_t   lastSelIMP[ 16];
   struct _mulle_objc_class      *lastIsa[ 16];
   struct _mulle_objc_class      *thisIsa;
   unsigned int                  i;
   void                          **sentinel;
   void                          *p;

   assert( mulle_objc_uniqueid_is_sane( methodid));
   memset( lastIsa, 0, sizeof( lastIsa));

   // assume compiler can do unrolling
   lookup   = _mulle_objc_class_lookup_implementation;
   sentinel = &objects[ n];

   while( objects < sentinel)
   {
      p = *objects++;
      if( ! p)
         continue;

      // our IMP cacheing thing
      thisIsa = _mulle_objc_object_get_isa( p);
      assert( thisIsa);

      i = ((uintptr_t) thisIsa >> sizeof( uintptr_t)) & 15;

      if( lastIsa[ i] != thisIsa)
      {
         imp            = (*lookup)( thisIsa, methodid);
         lastIsa[ i]    = thisIsa;
         lastSelIMP[ i] = imp;

         assert( imp);
      }

      mulle_objc_implementation_invoke( lastSelIMP[ i], p, methodid, params);
   }
}

# pragma mark -  call back and forth


// In a "regular" callchain, we want to have basically two scenarios:
// _init and _dealloc. With _init we want to initialize basics first and
// then progress to the more sophisticated categories in the back.
// With _dealloc, we want to go from back to front.
// As the saying is you go back and forth.. so init dealloc , back and forth
// The usual search direction is "back" to "front" so thats the usual movement.
//
// used by _init. overridden method come last
void   _mulle_objc_object_call_chained_back( void *obj,
                                             mulle_objc_methodid_t methodid,
                                             void *parameter)
{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_methodlist   *list;
   unsigned int                    inheritance;
   unsigned int                    once;
   mulle_objc_implementation_t     imp;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls         = _mulle_objc_object_get_isa( obj);
   inheritance = _mulle_objc_class_get_inheritance( cls);

   // i mean should we honor this if we explicitly want to to do
   // a call chain ? probably...
   once        = (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES);

   // enumerator forward
   mulle_concurrent_pointerarray_for( &cls->methodlists, list)
   {
      method = _mulle_objc_methodlist_search( list, methodid);
      if( method)
      {
         imp   = _mulle_objc_method_get_implementation( method);
         // this should use mulle_objc_implementation_invoke
         mulle_objc_implementation_invoke( imp, obj, methodid, parameter);
      }
      if( once)
         break;
   }
}


//
// used by _dealloc:  overridden method come first
//
// and probably also by other calls, since this is the natural direction
//
void   _mulle_objc_object_call_chained_forth( void *obj,
                                              mulle_objc_methodid_t methodid,
                                              void *parameter)
{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_methodlist   *list;
   unsigned int                    inheritance;
   unsigned int                    n;
   mulle_objc_implementation_t     imp;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls         = _mulle_objc_object_get_isa( obj);
   inheritance = _mulle_objc_class_get_inheritance( cls);

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   mulle_concurrent_pointerarray_for_reverse( &cls->methodlists, n, list)
   {
      method = _mulle_objc_methodlist_search( list, methodid);
      if( method)
      {
         imp = _mulle_objc_method_get_implementation( method);
         // this should use mulle_objc_implementation_invoke
         mulle_objc_implementation_invoke( imp, obj, methodid, parameter);
      }
   }
}


