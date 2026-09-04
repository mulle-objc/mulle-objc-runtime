//
// class-reuse-thread-noleak.m
//
// Reproduce the MulleObjC thread-local reuse alloc leak.
// The bug: mulle_objc_threadinfo_free() does _mulle__pointerarray_done() on
// reuseallocsperclassindex BEFORE calling foundation_destructor. But the
// foundation_destructor drains the autorelease pool which frees objects,
// triggering keep_alloc_thread to re-grow the already-done pointerarray.
// The new storage is then leaked.
//
// This test simulates this by installing a foundation_destructor that
// allocates and frees an instance after the pointerarray is already "done".
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-thread/mulle-thread.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
{
   double  values[4];   // make instance bigger than objectheader
}
+ (instancetype) alloc;
- (void) dealloc;
@end


@implementation Foo

+ (Class) class
{
   return( self);
}

+ (instancetype) alloc
{
   return( mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}

- (void) dealloc
{
   mulle_objc_instance_free( self);
}

@end


//
// This mimics what _mulle_objc_threadinfo_destructor does in MulleObjC:
// it drains pools which frees objects, triggering keep_alloc_thread
// AFTER the pointerarray has already been _done'd by mulle_objc_threadinfo_free.
//
static void   my_foundation_destructor( struct _mulle_objc_threadinfo *info,
                                        void *foundationspace)
{
   struct _mulle_objc_universe   *universe;
   mulle_thread_tss_t            threadkey;
   id                            obj;

   // Re-set TSS so that ObjC calls work (TSS was cleared before destructor fired).
   // This mimics what _mulle_objc_threadinfo_destructor does in MulleObjC.
   universe  = _mulle_objc_threadinfo_get_universe( info);
   threadkey = _mulle_objc_universe_get_threadkey( universe);
   mulle_thread_tss_set( threadkey, info);

   // This alloc+dealloc happens AFTER mulle_objc_threadinfo_free already
   // called _mulle__pointerarray_done on reuseallocsperclassindex.
   // The dealloc triggers keep_alloc_thread which regrows the pointerarray.
   // The new storage is never freed = LEAK.
   obj = [Foo alloc];
   [obj dealloc];

   // unregister from GC (like MulleObjC's resignas_universethread)
   _mulle_objc_thread_remove_universe_gc( universe);

   // Clear TSS again (like MulleObjC does)
   mulle_thread_tss_set( threadkey, NULL);
}


static mulle_thread_rval_t   thread_function( void *arg)
{
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_threadinfo  *threadinfo;
   id                             obj;
   int                            i;

   universe = mulle_objc_global_get_defaultuniverse();

   // register this thread with the runtime
   _mulle_objc_thread_register_universe_gc( universe);
   mulle_objc_thread_setup_threadinfo( universe);

   // install our foundation_destructor (simulating what MulleObjC does)
   threadinfo = _mulle_objc_thread_get_threadinfo( universe);
   threadinfo->foundation_destructor = my_foundation_destructor;

   // match allocators so reuse fires (infra->allocator must == threadinfo->objectallocator)
   // In a pure runtime test without foundation, infra->allocator is the universe allocator
   // but threadinfo->objectallocator is &mulle_default_allocator. They won't match unless
   // we force them to be the same.
   threadinfo->objectallocator = _mulle_objc_universe_get_allocator( universe);

   // allocate and free objects - this grows reuseallocsperclassindex
   for( i = 0; i < 100; i++)
   {
      obj = [Foo alloc];
      [obj dealloc];
   }

   // GC unregister happens in foundation_destructor (called from TSS destructor)
   // threadinfo is cleaned up by TSS destructor on thread exit:
   // 1. mulle_objc_threadinfo_free frees reuse entries + _mulle__pointerarray_done
   // 2. foundation_destructor fires (our function above) which triggers keep_alloc_thread
   // 3. keep_alloc_thread regrows the already-done pointerarray = LEAK
   return( 0);
}


int   main( void)
{
   mulle_thread_t   thread;
   int              rval;

   // start a secondary thread
   rval = mulle_thread_create( thread_function, NULL, &thread);
   if( rval)
   {
      fprintf( stderr, "mulle_thread_create failed\n");
      return( 1);
   }

   mulle_thread_join( thread);

   printf( "passed\n");
   return( 0);
}
