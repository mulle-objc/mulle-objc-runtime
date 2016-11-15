//
//  mulle_objc_class.c
//  mulle-objc
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
//
#include "mulle_objc_class.h"

#include "mulle_objc_ivar.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_call.h"
#include "mulle_objc_callqueue.h"
#include "mulle_objc_kvccache.h"
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


# pragma mark -
# pragma mark accessor

int   _mulle_objc_class_set_state_bit( struct _mulle_objc_class *cls, unsigned int bit)
{
   void   *state;
   void   *old;
   
   assert( bit);
   
   do
   {
      old   = _mulle_atomic_pointer_read( &cls->state);
      state = (void *) ((uintptr_t) old | bit);
      if( state == old)
         return( 0);
   }
   while( ! _mulle_atomic_pointer_compare_and_swap( &cls->state, state, old));

   return( 1);
}


int  _mulle_objc_class_set_placeholder( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_object *obj)
{
   assert( _mulle_objc_class_is_infraclass( cls));
   return( _mulle_atomic_pointer_compare_and_swap( &cls->placeholder.pointer, obj, NULL));
}


int  _mulle_objc_class_set_auxplaceholder( struct _mulle_objc_class *cls,
                                           struct _mulle_objc_object *obj)
{
   struct _mulle_objc_class   *meta;
      
   assert( _mulle_objc_class_is_infraclass( cls));
   meta = _mulle_objc_class_get_metaclass( cls);
   return( _mulle_atomic_pointer_compare_and_swap( &meta->placeholder.pointer, obj, NULL));
}


int  _mulle_objc_class_set_coderversion( struct _mulle_objc_class *cls,
                                         uintptr_t value)
{
   assert( _mulle_objc_class_is_infraclass( cls));
   return( _mulle_atomic_pointer_compare_and_swap( &cls->coderversion, (void *) value, NULL));
}


// 1: it has worked, 0: someone else was faster
int   _mulle_objc_class_set_taggedpointerindex( struct _mulle_objc_class *cls,
                                               unsigned int value)
{
   assert( _mulle_objc_class_is_infraclass( cls));
   return( _mulle_atomic_pointer_compare_and_swap( &cls->taggedpointerindex, (void *) (uintptr_t) value, NULL));
}


# pragma mark -
# pragma mark calls


static void   *_mulle_objc_call_class_waiting_for_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls)
{
   /* same thread ? we are single threaded! */
   if( _mulle_atomic_pointer_read( &cls->thread) != (void *) mulle_thread_self())
   {
      /* wait for other thread to finish with +initialize */
      while( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED))
         mulle_thread_yield();
   }
   
   return( mulle_objc_object_call_uncached_class( obj, methodid, parameter, cls));
}


void   *_mulle_objc_call_class_needs_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class    *meta;
   struct _mulle_objc_cache    *cache;
   struct _mulle_objc_method   *initialize;
   mulle_objc_cache_uint_t     n_entries;
   
   assert( mulle_objc_class_is_current_thread_registered( cls));
   
   //
   // An uninitalized class has the empty_cache as the cache. It also has
   // `cls->thread` NULL. This methods is therefore usually called twice
   // once for the meta class and once for the instance. Regardless in both
   // cases, it is checked if +initialize needs to run. But this is only
   // flagged in the meta class.
   //
   // If another thread enters here, it will expect `cls->thread` to be NULL.
   // If it isn't it waits for MULLE_OBJC_CACHE_INITIALIZED to go up.
   //
   // what is tricky is, that cls and metaclass are executing this
   // singlethreaded, but still cls and metacalss could be in different threads
   //
   
   if( ! _mulle_atomic_pointer_compare_and_swap( &cls->thread, (void *) mulle_thread_self(), NULL))
      return( _mulle_objc_call_class_waiting_for_cache( obj, methodid, parameter, cls));

   // Singlethreaded block with respect to cls, not meta though!
   {
      //
      // first do +initialize,  uncached execution
      // track state only in "meta" class
      //
      meta = _mulle_objc_class_is_metaclass( cls) ? cls : _mulle_objc_class_get_metaclass( cls);
      if( _mulle_objc_class_set_state_bit( meta, MULLE_OBJC_INITIALIZE_DONE))
      {
         // this stays "flat", don't grab code from superclass
         initialize = _mulle_objc_class_search_method( meta, MULLE_OBJC_INITIALIZE_METHODID, NULL,MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
         if( initialize)
         {
            (*_mulle_objc_method_get_implementation( initialize))( (struct _mulle_objc_object *) _mulle_objc_class_get_infraclass( meta), MULLE_OBJC_INITIALIZE_METHODID, NULL);
         }
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
      }
      else
      {
         cls->cachepivot.call2 = mulle_objc_object_call2_empty_cache;
         cls->call             = mulle_objc_object_call_class_empty_cache;
      }
   }
   
   // finally unfreze
   // threads waiting_for_cache will run now
   _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED);

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

# pragma mark -
# pragma mark consistency

int   _mulle_objc_class_is_sane( struct _mulle_objc_class *cls);

int   _mulle_objc_class_is_sane( struct _mulle_objc_class *cls)
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


# pragma mark -
# pragma mark initialization / deallocation

void   _mulle_objc_class_init( struct _mulle_objc_class *cls,
                               char *name,
                               size_t  instance_size,
                               mulle_objc_classid_t classid,
                               struct _mulle_objc_class *superclass,
                               struct _mulle_objc_runtime *runtime)
{
   extern void   *mulle_objc_call_needs_cache2( void *obj, mulle_objc_methodid_t methodid, void *parameter);
   
   assert( runtime);

   cls->name                     = name;
   
   cls->superclass               = superclass;
   cls->classid                  = classid;

   cls->instance_and_header_size = sizeof( struct _mulle_objc_objectheader) + instance_size;
   cls->call                     = _mulle_objc_call_class_needs_cache;
   cls->runtime                  = runtime;
   cls->inheritance              = runtime->classdefaults.inheritance;
   
   cls->cachepivot.call2         = mulle_objc_call_needs_cache2;

   _mulle_atomic_pointer_nonatomic_write( &cls->cachepivot.pivot.entries, runtime->empty_cache.entries);
   _mulle_atomic_pointer_nonatomic_write( &cls->kvc.entries, runtime->empty_cache.entries);
   
   _mulle_objc_fastmethodtable_init( &cls->vtab);
}


void    _mulle_objc_class_setup_pointerarrays( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_runtime *runtime)
{
   extern void   *mulle_objc_call_needs_cache2( void *obj, mulle_objc_methodid_t methodid, void *parameter);
   
   // initially room for 1 ivars list
   _mulle_concurrent_pointerarray_init( &cls->ivarlists, 1, &runtime->memory.allocator);
   
   // initially room for 16 categories
   _mulle_concurrent_pointerarray_init( &cls->methodlists, 16, &runtime->memory.allocator);
   
   // initially room for 8 protocols
   _mulle_concurrent_pointerarray_init( &cls->protocolids, 8, &runtime->memory.allocator);

   // initially room for 2 categories with properties
   _mulle_concurrent_pointerarray_init( &cls->propertylists, 2, &runtime->memory.allocator);
}


static void   _mulle_objc_class_done( struct _mulle_objc_class *class, struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache      *cache;
   
   assert( class);
   assert( allocator);
   
   cache = _mulle_objc_cachepivot_atomic_get_cache( &class->cachepivot.pivot);
   if( cache != &class->runtime->empty_cache)
      _mulle_objc_cache_free( cache, allocator);

   _mulle_objc_class_invalidate_all_kvcinfos( class);
      
   _mulle_concurrent_pointerarray_done( &class->ivarlists);
   _mulle_concurrent_pointerarray_done( &class->methodlists);
   _mulle_concurrent_pointerarray_done( &class->protocolids);
   _mulle_concurrent_pointerarray_done( &class->propertylists);
}


void   _mulle_objc_classpair_destroy( struct _mulle_objc_classpair *pair, struct mulle_allocator *allocator)
{
   assert( pair);
   assert( allocator);
   
   _mulle_concurrent_hashmap_done( &pair->infraclass.cvars);
   
   _mulle_objc_class_done( &pair->infraclass, allocator);
   _mulle_objc_class_done( &pair->metaclass, allocator);
   
   _mulle_allocator_free( allocator, pair);
}


void   mulle_objc_classpair_free( struct _mulle_objc_classpair *pair, struct mulle_allocator *allocator)
{
   if( ! pair)
      return;

   if( ! allocator)
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( __get_or_create_objc_runtime());
   }
   
   _mulle_objc_classpair_destroy( pair, allocator);
}


# pragma mark -
# pragma mark forwarding

static inline struct _mulle_objc_method   *_mulle_objc_class_search_forwardmethod( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_search_method( cls, MULLE_OBJC_FORWARD_METHODID, NULL, cls->inheritance);
   if( ! method)
      method = cls->runtime->classdefaults.forwardmethod;

   return( method);
}


struct _mulle_objc_method    *_mulle_objc_class_get_or_search_forwardmethod( struct _mulle_objc_class *cls)
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


struct _mulle_objc_method    *_mulle_objc_class_unfailing_get_or_search_forwardmethod( struct _mulle_objc_class *cls,
                                                                               mulle_objc_methodid_t   missing_method)
{
   struct _mulle_objc_method   *method;
   
   method = _mulle_objc_class_get_or_search_forwardmethod( cls);
   if( method)
      return( method);
   
   _mulle_objc_class_raise_method_not_found( cls, missing_method);
   return( NULL);  // compilers...
}


# pragma mark -
# pragma mark consistency check
static int  mulle_objc_classlists_are_sane( struct _mulle_objc_class *cls)
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


static int  mulle_objc_instanceclasslists_are_sane( struct _mulle_objc_class *cls)
{
   void   *storage;

   // need at least one possibly empty ivar list
   storage = _mulle_atomic_pointer_nonatomic_read( &cls->ivarlists.storage.pointer);
   if( ! storage || ! mulle_concurrent_pointerarray_get_count( &cls->ivarlists))
   {
      errno = ECHILD;
      return( 0);
   }
   
   // need at least one possibly empty property list
   storage = _mulle_atomic_pointer_nonatomic_read( &cls->propertylists.storage.pointer);
   if( ! storage || ! mulle_concurrent_pointerarray_get_count( &cls->propertylists))
   {
      errno = ECHILD;
      return( 0);
   }
   
   return( mulle_objc_classlists_are_sane( cls));
}

   
int   mulle_objc_class_is_sane( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *meta;
   
   if( ! _mulle_objc_class_is_sane( cls))
      return( 0);

   if( ! mulle_objc_instanceclasslists_are_sane( cls))
      return( 0);
   
   meta = _mulle_objc_class_get_metaclass( cls);
   if( ! meta)
   {
      errno = ECHILD;
      return( 0);
   }

   if( ! mulle_objc_classlists_are_sane( meta))
      return( 0);
   
   return( meta != cls ? _mulle_objc_class_is_sane( meta) : 1);
}


# pragma mark -
# pragma mark ivar lists

int   mulle_objc_class_add_ivarlist( struct _mulle_objc_class *cls,
                                      struct _mulle_objc_ivarlist *list)
{
   if( ! cls)
   {
      errno = EINVAL;
      return( -1);
   }
   
   if( ! list)
      list = &cls->runtime->empty_ivarlist;
   
   assert( ! _mulle_objc_class_is_metaclass( cls));
   _mulle_concurrent_pointerarray_add( &cls->ivarlists, list);
   return( 0);
}


void   mulle_objc_class_unfailing_add_ivarlist( struct _mulle_objc_class *cls,
                                                 struct _mulle_objc_ivarlist *list)
{
   if( mulle_objc_class_add_ivarlist( cls, list))
      _mulle_objc_runtime_raise_fail_errno_exception( cls->runtime);
}

# pragma mark -
# pragma mark ivars

//
// doesn't check for duplicates
//
struct _mulle_objc_ivar   *_mulle_objc_class_search_ivar( struct _mulle_objc_class *cls,
                                                          mulle_objc_ivarid_t ivarid)
{
   struct _mulle_objc_ivar                            *ivar;
   struct _mulle_objc_ivarlist                        *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                       n;
   
   n     = mulle_concurrent_pointerarray_get_count( &cls->ivarlists);
   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->ivarlists, n);
   
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      ivar = _mulle_objc_ivarlist_search( list, ivarid);
      if( ivar)
         return( ivar);
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
   
   if( ! cls->superclass)
      return( NULL);
   return( _mulle_objc_class_search_ivar( cls->superclass, ivarid));
}


struct _mulle_objc_ivar  *mulle_objc_class_search_ivar( struct _mulle_objc_class *cls,
                                                        mulle_objc_ivarid_t ivarid)
{
   assert( ivarid != MULLE_OBJC_NO_IVARID && ivarid != MULLE_OBJC_INVALID_IVARID);

   if( ! cls)
   {
      errno = EINVAL;
      return( NULL);
   }
   
   return( _mulle_objc_class_search_ivar( cls, ivarid));
}


# pragma mark -
# pragma mark properties
//
// doesn't check for duplicates
//
struct _mulle_objc_property   *_mulle_objc_class_search_property( struct _mulle_objc_class *cls,
                                                                  mulle_objc_propertyid_t propertyid)
{
   struct _mulle_objc_property                        *property;
   struct _mulle_objc_propertylist                    *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                       n;
   
   n     = mulle_concurrent_pointerarray_get_count( &cls->propertylists);
   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->propertylists, n);
   
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      property = _mulle_objc_propertylist_search( list, propertyid);
      if( property)
         return( property);
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
   
   if( ! cls->superclass)
      return( NULL);
   return( _mulle_objc_class_search_property( cls->superclass, propertyid));
}


struct _mulle_objc_property  *mulle_objc_class_search_property( struct _mulle_objc_class *cls,
                                                             mulle_objc_propertyid_t propertyid)
{
   assert( propertyid != MULLE_OBJC_NO_PROPERTYID && propertyid != MULLE_OBJC_INVALID_PROPERTYID);
   
   if( ! cls)
   {
      errno = EINVAL;
      return( NULL);
   }
   
   return( _mulle_objc_class_search_property( cls, propertyid));
}



int   mulle_objc_class_add_propertylist( struct _mulle_objc_class *cls,
                                         struct _mulle_objc_propertylist *list)
{
   struct _mulle_objc_propertylistenumerator   rover;
   struct _mulle_objc_property                 *property;
   mulle_objc_propertyid_t                     last;
   
   if( ! cls)
   {
      errno = EINVAL;
      return( -1);
   }
   
   assert( ! _mulle_objc_class_is_metaclass( cls));
   
   if( ! list)
      list = &cls->runtime->empty_propertylist;
   
   /* register instance methods */
   last  = MULLE_OBJC_MIN_UNIQUEID - 1;
   rover = _mulle_objc_propertylist_enumerate( list);
   while( property = _mulle_objc_propertylistenumerator_next( &rover))
   {
      assert( property->propertyid != MULLE_OBJC_NO_PROPERTYID && property->propertyid != MULLE_OBJC_INVALID_PROPERTYID);
      //
      // properties must be sorted by propertyid, so we can binary search them
      // (in the future)
      //
      if( last > property->propertyid)
      {
         errno = EDOM;
         return( -1);
      }
      last = property->propertyid;
   }
  
   _mulle_objc_propertylistenumerator_done( &rover);

   _mulle_concurrent_pointerarray_add( &cls->propertylists, list);
   return( 0);
}


void   mulle_objc_class_unfailing_add_propertylist( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_propertylist *list)
{
   if( mulle_objc_class_add_propertylist( cls, list))
      _mulle_objc_runtime_raise_fail_errno_exception( cls->runtime);
}


# pragma mark -
# pragma mark categories

static int   _mulle_objc_class_invalidate_methodcache( struct _mulle_objc_class *cls,
                                                       mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;
   struct _mulle_objc_cache        *cache;
   
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
   return( 0x1);
}


static int  invalidate_caches( struct _mulle_objc_runtime *runtime,
                               struct _mulle_objc_class *cls,
                               enum mulle_objc_runtime_type_t type,
                               char *key,
                               void *parent,
                              struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_methodlistenumerator   rover;
   struct _mulle_objc_method                 *method;
   
   // preferably nothing there yet
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED))
      return( mulle_objc_runtime_walk_ok);
   
   _mulle_objc_class_invalidate_all_kvcinfos( cls);
   
   // if caches have been cleaned for class, it's done
   rover = _mulle_objc_methodlist_enumerate( list);
   while( method = _mulle_objc_methodlistenumerator_next( &rover))
      if( _mulle_objc_class_invalidate_methodcache( cls, method->descriptor.methodid))
         break;
   _mulle_objc_methodlistenumerator_done( &rover);
   
   return( mulle_objc_runtime_walk_ok);
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



# pragma mark -
# pragma mark protocols

static char  footer[] = "(so it is not used as protocol class)\n";

static int  print_protocol( mulle_objc_protocolid_t protocolid,
                            struct _mulle_objc_class *cls,
                            void *userinfo)
{
   char   *s;
   
   s = _mulle_objc_runtime_search_debughashname( _mulle_objc_class_get_runtime( cls), protocolid);
   if( s)
      fprintf( stderr, "\t<%s>\n", s);
   else
      fprintf( stderr, "\t%08x\n", protocolid);
   return( 0);
}


//
// must be root, must conform to own protocol, must not have ivars
// must not conform to other protocols (it's tempting to conform to NSObject)
// If you conform to NSObject, NSObject methods will override your superclass(!)
//
int  mulle_objc_class_is_protocol_class( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_runtime   *runtime;
   
   if( ! cls)
      return( 0);
   
   runtime = _mulle_objc_class_get_runtime( cls);

   if( _mulle_objc_class_get_superclass( cls))
   {
      if( runtime->debug.warn.protocol_class)
         if( _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_runtime %p warning: %sclass \"%s\" matches a protocol of same name, but it is not a root class %s",
                    runtime,
                    _mulle_objc_class_is_metaclass( cls) ? "meta" : "", cls->name, footer);
      return( 0);
   }
   
   if( cls->instance_and_header_size > sizeof( struct _mulle_objc_objectheader))
   {
      if( runtime->debug.warn.protocol_class)
         if( _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_runtime %p warning: %sclass \"%s\" matches a protocol of the same name"
                 ", but implements instance variables %s",
                   runtime,
                    _mulle_objc_class_is_metaclass( cls) ? "meta" : "", cls->name, footer);
      return( 0);
   }
   
   if( ! _mulle_objc_class_conforms_to_protocol( cls, cls->classid))
   {
      if( runtime->debug.warn.protocol_class)
         if( _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_WARN_PROTOCOL))
            fprintf( stderr, "mulle_objc_runtime %p warning: %sclass \"%s\" matches a protocol but does not conform to it %s",
                    runtime,
                    _mulle_objc_class_is_metaclass( cls) ? "meta" : "", cls->name, footer);
      return( 0);
   }
   
   //
   //
   //
   if( mulle_concurrent_pointerarray_get_count( &cls->protocolids) != 1)
   {
      int   is_NSObject;
      
      is_NSObject = ! strcmp( cls->name, "NSObject");
      
      if( is_NSObject || runtime->debug.warn.protocol_class)
      {
         if( _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_WARN_PROTOCOL))
         {
            fprintf( stderr, "mulle_objc_runtime %p warning: %sclass \"%s\" conforms to a protocol but also conforms to other protocols %s",
                    runtime,
                    _mulle_objc_class_is_metaclass( cls) ? "meta" : "", cls->name, footer);
         
            fprintf( stderr, "Protocols:\n");
            _mulle_objc_class_walk_protocolids( cls, print_protocol, NULL);
         }
         if( is_NSObject)
            _mulle_objc_runtime_raise_inconsistency_exception( runtime, "multiple protocols on NSObject is fatal for #mulle_objc");
      }
      
      return( 0);
   }
   
   return( 1);
}


void   _mulle_objc_class_add_protocol( struct _mulle_objc_class *cls,
                                       mulle_objc_protocolid_t uniqueid)
{
   assert( cls);
   assert( uniqueid != MULLE_OBJC_NO_PROTOCOLID);
   assert( uniqueid != MULLE_OBJC_INVALID_PROTOCOLID);
   
   _mulle_concurrent_pointerarray_add( &cls->protocolids, (void *) (uintptr_t) uniqueid);
}


void   _mulle_objc_class_raise_null_exception( void)
{
   struct _mulle_objc_runtime   *runtime;
   
   runtime = __get_or_create_objc_runtime();
   errno = EINVAL;
   _mulle_objc_runtime_raise_fail_errno_exception( runtime);
}


void   mulle_objc_class_unfailing_add_protocols( struct _mulle_objc_class *cls,
                                                 mulle_objc_protocolid_t *protocolids)
{
   mulle_objc_protocolid_t   uniqueid;
   
   if( ! cls)
      _mulle_objc_class_raise_null_exception();
   
   if( ! protocolids)
      return;
      
   while( uniqueid = *protocolids++)
   {
      _mulle_concurrent_pointerarray_add( &cls->protocolids, (void *) (uintptr_t) uniqueid);
   }
}


int   _mulle_objc_class_conforms_to_protocol( struct _mulle_objc_class *cls, mulle_objc_protocolid_t protocolid)
{
   int   rval;
   
   rval = _mulle_concurrent_pointerarray_find( &cls->protocolids, (void *) (uintptr_t) protocolid);
   if( rval)
      return( rval);

   if( ! (cls->inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( cls->superclass && cls->superclass != cls)
      {
         rval = _mulle_objc_class_conforms_to_protocol( cls->superclass, protocolid);
         if( rval)
            return( rval);
      }
   }
   
   /* should query protocols too ? */
   
   return( rval);
}



struct _mulle_objc_class  *_mulle_objc_protocolclassenumerator_next( struct _mulle_objc_protocolclassenumerator *rover)
{
   struct _mulle_objc_class     *cls;
   struct _mulle_objc_class     *proto_cls;
   struct _mulle_objc_runtime   *runtime;
   mulle_objc_protocolid_t      uniqueid;
   void                         *value;
   
   proto_cls  = NULL;

   cls     = _mulle_objc_protocolclassenumerator_get_class( rover);
   runtime = _mulle_objc_class_get_runtime( cls);
   
   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover->list_rover))
   {
      uniqueid = (mulle_objc_protocolid_t) (uintptr_t) value;
      if( rover->cls->classid == uniqueid)  // don't recurse into self
         continue;
      
      proto_cls = _mulle_objc_runtime_lookup_class( runtime, uniqueid);
      if( ! mulle_objc_class_is_protocol_class( proto_cls))
         continue;
      
      if( _mulle_objc_class_is_metaclass( cls))
         proto_cls = _mulle_objc_class_get_metaclass( proto_cls);
      break;
   }
   return( proto_cls);
}


static int   _mulle_objc_class_protocol_walk_methods( struct _mulle_objc_class *cls,
                                                      unsigned int inheritance,
                                                      int (*f)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *),
                                                      void *userinfo)
{
   int                                          rval;
   struct _mulle_objc_class                     *proto_cls;
   struct _mulle_objc_protocolclassenumerator   rover;
   
   rval  = 0;
   rover = _mulle_objc_class_enumerate_protocolclasses( cls);
   while( proto_cls = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      if( proto_cls == cls)
         continue;
   
      if( rval = _mulle_objc_class_walk_methods( proto_cls, inheritance | proto_cls->inheritance, f, userinfo))
         break;
   }
   _mulle_objc_protocolclassenumerator_done( &rover);
   
   return( rval);
}


static struct _mulle_objc_method  *_mulle_objc_class_protocol_search_method( struct _mulle_objc_class *cls,
                                                                             mulle_objc_methodid_t methodid,
                                                                             struct _mulle_objc_method *previous,
                                                                            unsigned int inheritance)
{
   struct _mulle_objc_class                     *proto_cls;
   struct _mulle_objc_protocolclassenumerator   rover;
   struct _mulle_objc_method                    *found;
   struct _mulle_objc_method                    *method;

   found = NULL;
   
   rover = _mulle_objc_class_enumerate_protocolclasses( cls);
   while( proto_cls = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      if( proto_cls == cls)
         continue;
      
      method = _mulle_objc_class_search_method( proto_cls, methodid, previous, inheritance | proto_cls->inheritance);
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


static unsigned int   _mulle_objc_class_protocols_count_preload_methods( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class                     *proto_cls;
   struct _mulle_objc_protocolclassenumerator   rover;
   unsigned int                                 preloads;
   
   preloads = 0;

   rover = _mulle_objc_class_enumerate_protocolclasses( cls);
   while( proto_cls = _mulle_objc_protocolclassenumerator_next( &rover))
   {
      if( proto_cls == cls)
         continue;
      preloads += _mulle_objc_class_count_preload_methods( proto_cls);
   }
   _mulle_objc_protocolclassenumerator_done( &rover);
   
   return( preloads);
}


# pragma mark -
# pragma mark methods

unsigned int   _mulle_objc_class_count_preload_methods( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *dad;
   unsigned int               preloads;
   
   preloads = cls->preloads;
   if( ! (cls->inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      dad = cls;
      while( dad = dad->superclass)
         preloads += _mulle_objc_class_count_preload_methods( dad);
   }
   
   if( ! (cls->inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
      preloads += _mulle_objc_class_protocols_count_preload_methods( cls);

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


typedef   int (*mulle_objc_walk_ivars_callback)( struct _mulle_objc_ivar *, struct _mulle_objc_class *, void *);

int   _mulle_objc_class_walk_ivars( struct _mulle_objc_class *cls, unsigned int inheritance , mulle_objc_walk_ivars_callback f, void *userinfo)
{
   int                                               rval;
   struct _mulle_objc_ivarlist                       *list;
   struct mulle_concurrent_pointerarrayreverseenumerator  rover;
   unsigned int                                      n;
   
   // todo: need to lock class
   
   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category
   
   n = mulle_concurrent_pointerarray_get_count( &cls->ivarlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;
   
   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->ivarlists, n);
   
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( rval = _mulle_objc_ivarlist_walk( list, f, cls, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( cls->superclass && cls->superclass != cls)
         return( _mulle_objc_class_walk_ivars( cls->superclass, inheritance, f, userinfo));
   }
   
   return( 0);
}


typedef   int (*mulle_objc_walk_properties_callback)( struct _mulle_objc_property *, struct _mulle_objc_class *, void *);

int   _mulle_objc_class_walk_properties( struct _mulle_objc_class *cls, unsigned int inheritance , mulle_objc_walk_properties_callback f, void *userinfo)
{
   int                                                rval;
   struct _mulle_objc_propertylist                    *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                       n;
   
   // todo: need to lock class
   
   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category
   
   n = mulle_concurrent_pointerarray_get_count( &cls->propertylists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;
   
   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->propertylists, n);
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      if( rval = _mulle_objc_propertylist_walk( list, f, cls, userinfo))
         return( rval);
   }
   
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( cls->superclass && cls->superclass != cls)
         return( _mulle_objc_class_walk_properties( cls->superclass, inheritance, f, userinfo));
   }
   
   return( 0);
}


int   _mulle_objc_class_walk_protocolids( struct _mulle_objc_class *cls,
                                          int (*f)( mulle_objc_protocolid_t, struct _mulle_objc_class *, void *),
                                          void *userinfo)
{
   int                                         rval;
   mulle_objc_propertyid_t                     propertyid;
   struct mulle_concurrent_pointerarrayenumerator   rover;
   void                                        *value;
   
   rover = mulle_concurrent_pointerarray_enumerate( &cls->protocolids);
   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      propertyid = (mulle_objc_propertyid_t) (uintptr_t) value; // fing warning
      if( rval = (*f)( propertyid, cls, userinfo))
      {
         if( rval < 0)
            errno = ENOENT;
         return( rval);
      }
   }
   return( 0);
}


//
// if previous is set, the search will be done for the method that previous
// has overriden
//
struct _mulle_objc_method   *_mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid,
                                                              struct _mulle_objc_method *previous,
                                                              unsigned int inheritance)
{
   struct _mulle_objc_runtime                         *runtime;
   struct _mulle_objc_method                          *found;
   struct _mulle_objc_method                          *method;
   struct _mulle_objc_methodlist                      *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                       n;
   unsigned int                                       tmp;
   
   assert( mulle_objc_class_is_current_thread_registered( cls));

   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;
   
   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);
   found = NULL;
   
   runtime = _mulle_objc_class_get_runtime( cls);
   if( runtime->debug.trace.method_searches)
      fprintf( stderr, "mulle_objc_runtime %p trace: search class %s for methodid %08x (previous=%p)\"\n", runtime, cls->name, methodid, previous ? _mulle_objc_method_get_implementation( previous) : NULL);
   
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
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
            {
               // one more ? it's a category
               if( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
                  fprintf( stderr, "mulle_objc_runtime %p trace: found in category %s( %s) implementation %p for methodid %08x ( \"%s\")\"\n", runtime, cls->name, list->owner ? list->owner : "", _mulle_objc_method_get_implementation( method), method->descriptor.methodid, method->descriptor.name);
               else
                  fprintf( stderr, "mulle_objc_runtime %p trace: found in class %s implementation %p for methodid %08x ( \"%s\")\"\n", runtime, cls->name, _mulle_objc_method_get_implementation( method), method->descriptor.methodid, method->descriptor.name);
            }

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
      
      method = _mulle_objc_class_protocol_search_method( cls, methodid, previous, tmp);
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
   
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( cls->superclass)
      {
         method = _mulle_objc_class_search_method( cls->superclass, methodid, previous, cls->superclass->inheritance);
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
   
   method = _mulle_objc_class_search_method( cls, methodid, NULL, cls->inheritance);
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
   
   method = _mulle_objc_class_search_method( cls, methodid, NULL, cls->inheritance);
   if( ! method && errno == EEXIST)
      _mulle_objc_runtime_raise_inconsistency_exception( cls->runtime, "class \"%s\" hidden method override of %llx", _mulle_objc_class_get_name( cls), methodid);
   
   return( method);
}
