//
//  test_retain_release.c
//  mulle-objc-runtime
//
//  Created by Nat! on 14.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include "test_runtime_ids.h"


static intptr_t   get_retaincount( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   header = _mulle_objc_object_get_objectheader( obj);
   return( _mulle_objc_objectheader_get_retaincount_1( header));
}



static unsigned int   instances;

static void  *my_calloc( size_t n, size_t size)
{
   ++instances;
   return( mulle_allocator_calloc( &mulle_testallocator, n, size));
}


static void  my_free( void *p)
{
   --instances;
   mulle_allocator_free( &mulle_testallocator, p);
}


static void  my_nop_free( void *p)
{
   --instances;
}


struct mulle_allocator  my_allocator =
{
   my_calloc,
   NULL,
   my_free,
};


static void  *A_alloc( struct _mulle_objc_infraclass *self, mulle_objc_methodid_t _cmd)
{
   return( _mulle_objc_infraclass_allocator_alloc_instance_extra( self, 0, &my_allocator));
}

struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods; // must be #0 and same as struct _mulle_objc_ivarlist
   void                        *owner;
   struct _mulle_objc_method   methods[];
};


struct _gnu_mulle_objc_methodlist   A_infra_methodlist =
{
   2,
   NULL,
   {
      {
         {
            MULLE_OBJC_RELEASE_METHODID,
            "v@:",
            "release",
            0
         },
         (void *) mulle_objc_object_release
      },
      {
         {
            MULLE_OBJC_RETAIN_METHODID,
            "@@:",
            "retain",
            0
         },
         (void *) mulle_objc_object_retain
      },
   }
};


struct _gnu_mulle_objc_methodlist   A_meta_methodlist =
{
   1,
   NULL,
   {
      {
         {
            MULLE_OBJC_ALLOC_METHODID,
            "v@:",
            "alloc",
            0
         },
         (void *) A_alloc
      }
   }
};



static void   test_simple_retain_release( struct _mulle_objc_infraclass *A_infra)
{
   struct _mulle_objc_object   *a;
   struct _mulle_objc_object   *b;

   assert( instances == 0);
   a = _mulle_objc_infraclass_allocator_alloc_instance_extra( A_infra, 0, &my_allocator);
   assert( instances == 1);
   assert( mulle_objc_object_get_retaincount( a) == 1);

   b = mulle_objc_object_retain( a);
   assert( a == b);
   assert( mulle_objc_object_get_retaincount( a) == 2);

   mulle_objc_object_release( a);
   assert( mulle_objc_object_get_retaincount( a) == 1);

   __mulle_objc_instance_free( a, &my_allocator);
   assert( instances == 0);
}


static void   test_permanent_retain_release( struct _mulle_objc_infraclass *A_infra)
{
   struct _mulle_objc_object   *a;
   long                        retain_count;

   a = _mulle_objc_infraclass_allocator_alloc_instance_extra( A_infra, 0, &my_allocator);
   _mulle_objc_object_constantify_noatomic( a);

   retain_count = mulle_objc_object_get_retaincount( a);
   mulle_objc_object_retain( a);
   assert( mulle_objc_object_get_retaincount( a) == retain_count);

   mulle_objc_object_release( a);
   assert( mulle_objc_object_get_retaincount( a) == retain_count);

   mulle_objc_object_release( a);
   assert( mulle_objc_object_get_retaincount( a) == retain_count);
   assert( instances == 1);

   __mulle_objc_instance_free( a, &my_allocator);
   assert( instances == 0);
}


# pragma mark - dealloc

static unsigned int   dealloced;
static unsigned int   finalized;

static void    A_finalize( void *self, mulle_objc_classid_t sel)
{
   assert( get_retaincount( self) < -1);
   ++finalized;
}


static void    A_dealloc( void *self, mulle_objc_classid_t sel)
{
   assert( get_retaincount( self) == -1);
   ++dealloced;
   __mulle_objc_instance_free( self, &my_allocator);
}


static struct _gnu_mulle_objc_methodlist   finalize_dealloc_methodlist =
{
   2,
   NULL,
   {
      {
         {
            MULLE_OBJC_DEALLOC_METHODID,
            "@:",
            "dealloc",
            0
         },
         (void *) A_dealloc
      },
      {
         {
            MULLE_OBJC_FINALIZE_METHODID,
            "@:",
            "finalize",
            0
         },
         (void *) A_finalize
      }
   }
};


static void   test_dealloc_finalize( struct _mulle_objc_infraclass  *A_infra)
{
   struct _mulle_objc_object   *a;

   assert( dealloced == 0);

   mulle_objc_infraclass_add_methodlist_nofail( A_infra, (void *) &finalize_dealloc_methodlist);

   a = _mulle_objc_infraclass_allocator_alloc_instance_extra( A_infra, 0, &my_allocator);

   mulle_objc_object_release( a);
   assert( finalized == 1);
   assert( dealloced == 1);

   dealloced = 0;
   finalized = 0;
}


static void   test_perform_finalize( struct _mulle_objc_infraclass  *A_infra)
{
   struct _mulle_objc_object   *a;

   assert( dealloced == 0);

   a = _mulle_objc_infraclass_allocator_alloc_instance_extra( A_infra, 0, &my_allocator);
   assert( get_retaincount( a) == 0);

   mulle_objc_object_perform_finalize( a);
   assert( get_retaincount( a) < -1);
   assert( finalized == 1);
   assert( dealloced == 0);

   mulle_objc_object_release( a);
   assert( dealloced == 1);
   assert( finalized == 1);

   dealloced = 0;
   finalized = 0;
}


void   test_retain_release( void)
{
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_infraclass   *A_infra;
   struct _mulle_objc_metaclass    *A_meta;
   struct _mulle_objc_universe     *universe;

   universe = mulle_objc_global_register_universe( MULLE_OBJC_DEFAULTUNIVERSEID, NULL);

   pair = mulle_objc_universe_new_classpair( universe, A_classid, "A", 0, 0, NULL);
   assert( pair);
   A_infra = _mulle_objc_classpair_get_infraclass( pair);
   A_meta = _mulle_objc_classpair_get_metaclass( pair);

   mulle_objc_infraclass_add_methodlist_nofail( A_infra, (void *) &A_infra_methodlist);
   mulle_objc_metaclass_add_methodlist_nofail( A_meta, (void *) &A_meta_methodlist);
   mulle_objc_infraclass_add_ivarlist_nofail( A_infra, NULL);
   mulle_objc_infraclass_add_propertylist_nofail( A_infra, NULL);

   mulle_objc_universe_add_infraclass_nofail( universe, A_infra);

   test_simple_retain_release( A_infra);
   test_permanent_retain_release( A_infra);

   test_dealloc_finalize( A_infra);
   test_perform_finalize( A_infra);
}
