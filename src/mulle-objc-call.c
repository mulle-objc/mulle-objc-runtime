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
#include "mulle-objc-methodcache.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"

#include "include-private.h"

#include <assert.h>
#include <stdlib.h>


// this is needed so we can call tracing (at least in -O0 situations)
MULLE_C_NEVER_INLINE static void *
   _mulle_objc_object_call_class_nofail( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter,
                                         struct  _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   mulle_objc_implementation_t   imp;

   imp      = _mulle_objc_class_lookup_implementation_nofail( cls, methodid);
   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_call)
      mulle_objc_class_trace_call( cls, obj, methodid, parameter, imp);

   /*->*/
   return( (*imp)( obj, methodid, parameter));
}


#pragma mark - non-inline API calls



// we can't do the FCS code here, because its just super slow if
// the constant isn't known.
// This is like call2, except that we need do check the first
// cache entry.
MULLE_C_NEVER_INLINE
void   *mulle_objc_object_call( void *obj,
                                mulle_objc_methodid_t methodid,
                                void *parameter)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_class         *cls;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   do
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
/*->*/
         return( (*imp)( obj, methodid, parameter));
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   while( entry->key.uniqueid);
/*->*/

   return( _mulle_objc_object_call_class_nofail( obj, methodid, parameter, cls));
}


// as above but not obj check
MULLE_C_NEVER_INLINE
void   *_mulle_objc_object_call( void *obj,
                                 mulle_objc_methodid_t methodid,
                                 void *parameter)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_class         *cls;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   do
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
/*->*/
         return( (*imp)( obj, methodid, parameter));
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   while( entry->key.uniqueid);
/*->*/
   return( _mulle_objc_object_call_class_nofail( obj, methodid, parameter, cls));
}



MULLE_C_NEVER_INLINE
void   *mulle_objc_object_supercall( void *obj,
                                     mulle_objc_methodid_t methodid,
                                     void *parameter,
                                     mulle_objc_superid_t superid)
{
   return( mulle_objc_object_supercall_inline( obj, methodid, parameter, superid));
}



# pragma mark - normal callbacks for memorycache


static void   *_mulle_objc_object_call_class( void *obj,
                                              mulle_objc_methodid_t methodid,
                                              void *parameter,
                                              struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( obj);
   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];

      if( entry->key.uniqueid == methodid)
      {
         p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp     = (mulle_objc_implementation_t) p;
/*->*/   return( (*imp)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
         break;

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
/*->*/

   return( _mulle_objc_object_call_class_nofail( obj, methodid, parameter, cls));
}


//
// this function is called, when the first inline cache check gave a
// collision, it skips the first found entry. This method is put into
// the method cache, you don't call it directly.
//
static void   *_mulle_objc_object_call2( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_class         *cls;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   do
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp     = (mulle_objc_implementation_t) p;
/*->*/
         return( (*imp)( obj, methodid, parameter));
      }
   }
   while( entry->key.uniqueid);
/*->*/

   return( _mulle_objc_object_call_class_nofail( obj, methodid, parameter, cls));
}


//
// This function is called, when the first inline cache check gave a
// collision, it skips the first found entry. This is not called directly
// but placed into the method cache.
//
static mulle_objc_implementation_t
   _mulle_objc_class_superlookup2_implementation_nofail( struct _mulle_objc_class *cls,
                                                         mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) superid;
   do
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == superid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
/*->*/   return( imp);
      }
   }
   while( entry->key.uniqueid);
/*->*/
   imp = _mulle_objc_class_superrefresh_implementation_nofail( cls, superid);
   // TODO: this doesn't trace yet
   return( imp);
}


void  _mulle_objc_methodcache_init_normal_callbacks( struct _mulle_objc_methodcache *p)
{
   p->call         = _mulle_objc_object_call_class;
   p->call2        = _mulle_objc_object_call2;
   p->superlookup  = _mulle_objc_class_superlookup_implementation_nofail;  // public actually
   p->superlookup2 = _mulle_objc_class_superlookup2_implementation_nofail;
}


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

      // TODO: this doesn't trace yet
      (*lastSelIMP[ i])( p, methodid, params);
   }
}


# pragma mark - trace method

void   mulle_objc_class_trace_call( struct _mulle_objc_class *cls,
                                    void *obj,
                                    mulle_objc_methodid_t methodid,
                                    void *parameter,
                                    mulle_objc_implementation_t imp)
{
   struct _mulle_objc_descriptor        *desc;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_method            *method;
   struct _mulle_objc_class             *isa;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   unsigned int                         inheritance;
   char                                 *name;
   int                                  frames;

   universe = _mulle_objc_class_get_universe( cls);
   //
   // What is basically wrong here is, that we should be searching for the
   // class that implements the method, not the called class
   // This is fairly expensive though...
   //
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


