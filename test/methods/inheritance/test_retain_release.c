//
//  test_retain_release.c
//  mulle-objc-runtime
//
//  Created by Nat! on 14.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-test-allocator/mulle-test-allocator.h>

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
   return( mulle_allocator_calloc( &mulle_test_allocator, n, size));
}


static void  my_free( void *p)
{
   --instances;
   mulle_allocator_free( &mulle_test_allocator, p);
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
   return( __mulle_objc_infraclass_alloc_instance_extra( self, 0, &my_allocator));
}


struct _mulle_objc_methodlist   A_alloc_methodlist =
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
   a = __mulle_objc_infraclass_alloc_instance_extra( A_infra, 0, &my_allocator);
   assert( instances == 1);
   assert( mulle_objc_object_get_retaincount( a) == 1);

   b = mulle_objc_object_retain( a);
   assert( a == b);
   assert( mulle_objc_object_get_retaincount( a) == 2);

   mulle_objc_object_release( a);
   assert( mulle_objc_object_get_retaincount( a) == 1);

   __mulle_objc_object_free( a, &my_allocator);
   assert( instances == 0);
}


static void   test_permanent_retain_release( struct _mulle_objc_infraclass *A_infra)
{
   struct _mulle_objc_object   *a;
   long                        retain_count;

   a = __mulle_objc_infraclass_alloc_instance_extra( A_infra, 0, &my_allocator);
   _mulle_objc_object_infiniteretain_noatomic( a);

   retain_count = mulle_objc_object_get_retaincount( a);
   mulle_objc_object_retain( a);
   assert( mulle_objc_object_get_retaincount( a) == retain_count);

   mulle_objc_object_release( a);
   assert( mulle_objc_object_get_retaincount( a) == retain_count);

   mulle_objc_object_release( a);
   assert( mulle_objc_object_get_retaincount( a) == retain_count);
   assert( instances == 1);

   __mulle_objc_object_free( a, &my_allocator);
   assert( instances == 0);
}


# pragma mark -
# pragma mark dealloc

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
   __mulle_objc_object_free( self, &my_allocator);
}


struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods; // must be #0 and same as struct _mulle_objc_ivarlist
   void                        *owner;
   struct _mulle_objc_method   methods[];
};


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

   a = __mulle_objc_infraclass_alloc_instance_extra( A_infra, 0, &my_allocator);

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

   a = __mulle_objc_infraclass_alloc_instance_extra( A_infra, 0, &my_allocator);
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

   pair = mulle_objc_new_classpair_nofail( A_classid, "A", 0, NULL);
   assert( pair);
   A_infra = _mulle_objc_classpair_get_infraclass( pair);
   A_meta = _mulle_objc_classpair_get_metaclass( pair);

   mulle_objc_infraclass_add_methodlist_nofail( A_infra, NULL);
   mulle_objc_metaclass_add_methodlist_nofail( A_meta, NULL);
   mulle_objc_infraclass_add_ivarlist_nofail( A_infra, NULL);
   mulle_objc_infraclass_add_propertylist_nofail( A_infra, NULL);

   mulle_objc_add_infraclass_nofail( A_infra);

   test_simple_retain_release( A_infra);
   test_permanent_retain_release( A_infra);

   test_dealloc_finalize( A_infra);
   test_perform_finalize( A_infra);
}
