//
//  main.c
//  test-runtime-2
//
//  Created by Nat! on 19/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_NO_TPS__
# define __MULLE_OBJC_FCS__
# if defined( DEBUG) || ! defined( __OPTIMIZE__)
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif



#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


#define ___Foo_classid         MULLE_OBJC_CLASSID( 0xc7e16770)

#define ___a___methodid        MULLE_OBJC_METHODID( 0x40c292ce)
#define ___init__methodid      MULLE_OBJC_INIT_METHODID
#define ___setA___methodid     MULLE_OBJC_METHODID( 0xffb5e54a)

#define ___a___ivarid          MULLE_OBJC_IVARID( 0x40c292ce)


struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods; // must be #0 and same as struct _mulle_objc_ivarlist
   void                        *owner;
   struct _mulle_objc_method   methods[];
};


struct _gnu_mulle_objc_ivarlist
{
   unsigned int              n_ivars;  // must be #0 and same as struct _mulle_objc_methodlist

   struct _mulle_objc_ivar   ivars[];
};


struct _gnu_mulle_objc_loadclasslist
{
   unsigned int                    n_loadclasses;
   struct _mulle_objc_loadclass    *loadclasses[];
};



/* in this example, Foo inherits from Object and has a category


   @interface Foo
   {
      int  a;
   }
   - (void) setA:(int) a;
   - (int) a;
   @end

   @implementation Foo
   - (void *) init
   {
      self->a = 1;
      return( self);
   }
   - (void) setA:(int) a
   {
      self->a = a;
   }
   - (int) a
   {
      return( a);
   }
   @end


   int   main( int argc, const char * argv[])
   {
      Foo  *obj;

      obj = [[Foo alloc] init];

      return 0;
   }
  */

// @interface Foo : Object

#define __Foo_iVARs  \

struct Foo
{
   int   a;
};

// @end

// @implementation Foo
//
//- (void *) init
//{
//   a = 1;
//   b = 2;
//
//   return( self);
//}
//

static void   *Foo_init( struct Foo *self, mulle_objc_methodid_t _cmd, void *_params)
{
   self->a = 1;

   return( self);
}


// - (void) setA:(int) a b:(int) b
// {
//    self->a = a;
//    self->b = b;
// }

//
// figure out if we can get the compiler to alias a  with _params->_a with
// "const"ing
//
static void   Foo_setA_b_( struct Foo *self, mulle_objc_methodid_t _cmd, void *_params)
{
   int   a = (int) (intptr_t) _params;

   self->a = a;
}


static void   *Foo_a( struct Foo *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( (void *) (intptr_t) self->a);
}

// @end


static struct _gnu_mulle_objc_ivarlist  Foo_ivarlist =
{
   1,
   // must be sorted by ivarid !!!
   {
      {
         {
            ___a___ivarid,
            "a",
            "i"
         },
         offsetof( struct Foo, a)
      }
   }
};


static struct _gnu_mulle_objc_methodlist  Foo_instance_methodlist =
{
   3,
   NULL,
   {
      {
         // idee make this "197380f3\0setA:b:\0@:ii" as a uniquable string
         // also saving an additional 2 pointers for method definition
         // but what if the type differs ?
         {
            ___a___methodid,
            "@:",
            "a",
            0
         },
         (void *) Foo_setA_b_
      },
      {
         {
            ___init__methodid,
            "@:",
            "init",
            0
         },
         (void *) Foo_init
      },
      {
         // idee make this "197380f3\0setA:b:\0@:ii" as a uniquable string
         // also saving an additional 2 pointers for method definition
         // but what if the type differs ?
         {
            ___setA___methodid,
            "@:ii",
            "setA:",
            0
         },
         (void *) Foo_setA_b_
      }
   }
};


static struct _mulle_objc_loadclass  Foo_loadclass =
{
   ___Foo_classid,
   "Foo",
   0,

   0,
   NULL,
   0,

   -1,
   sizeof( struct Foo),

   (struct _mulle_objc_ivarlist *)  &Foo_ivarlist,
   NULL,
   (struct _mulle_objc_methodlist *) &Foo_instance_methodlist,
   NULL,

   NULL
};



struct _gnu_mulle_objc_loadclasslist  class_list =
{
   1,
   {
      &Foo_loadclass
   }
};




#ifdef __MULLE_OBJC_NO_TPS__
# define TPS_BIT   0x4
#else
# define TPS_BIT   0
#endif

#ifdef __MULLE_OBJC_NO_FCS__
# define FCS_BIT   0x8
#else
# define FCS_BIT   0
#endif

#ifdef __MULLE_OBJC_TAO__
# define TAO_BIT   0x10
#else
# define TAO_BIT   0
#endif



static struct _mulle_objc_loadinfo  load_info =
{
   {
      MULLE_OBJC_RUNTIME_LOAD_VERSION,
      MULLE_OBJC_RUNTIME_VERSION,
      0,
      0,
      TPS_BIT | FCS_BIT | TAO_BIT
   },
   NULL,
   (struct _mulle_objc_loadclasslist *) &class_list,  // let runtime sort for us
   NULL,
   NULL,
   NULL,
};



MULLE_C_CONSTRUCTOR( __load)
static void  __load()
{
   static int  has_loaded;

   fprintf( stderr, "--> __load\n");

   // windows w/o mulle-clang
   if( has_loaded)
      return;
   has_loaded = 1;

   mulle_objc_loadinfo_enqueue_nofail( &load_info);
}


struct _mulle_objc_universe  *
   __register_mulle_objc_universe( mulle_objc_universeid_t universeid,
                                   char *universename)
{
   struct _mulle_objc_universe    *universe;

   universe = __mulle_objc_global_get_universe( universeid, universename);
   if( ! _mulle_objc_universe_is_initialized( universe))
   {
      _mulle_objc_universe_bang( universe, 0, NULL, NULL);
      universe->config.ignore_ivarhash_mismatch = 1;
   }
   return( universe);
}


struct _mulle_objc_kvcinfo   *infos[ 32];


char   *keys[ 32] =
{
   "a",
   "aa",
   "aaa",
   "aaaa",
   "aaaaa",
   "aaaaaa",
   "aaaaaaa",
   "aaaaaaaa",

   "b",
   "bb",
   "bbb",
   "bbbb",
   "bbbbb",
   "bbbbbb",
   "bbbbbbb",
   "bbbbbbbb",

   "c",
   "cc",
   "ccc",
   "cccc",
   "ccccc",
   "cccccc",
   "ccccccc",
   "cccccccc",

   "d",
   "dd",
   "ddd",
   "dddd",
   "ddddd",
   "dddddd",
   "ddddddd",
   "dddddddd"
};


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass    *infra;
   struct _mulle_objc_class         *cls;
   struct _mulle_objc_object        *obj;
   struct _mulle_objc_kvcinfo       *p;
   struct mulle_allocator           *allocator;
   unsigned int                     n_sets;
   unsigned int                     n_gets;
   unsigned int                     i;
   int                              rval;

   // windows...
#if ! defined( __clang__) && ! defined( __GNUC__)
   __load();
#endif

   // obj = [[Foo alloc] init];

   infra = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID, ___Foo_classid);
   obj = mulle_objc_infraclass_alloc_instance( infra);
   obj = (void *) mulle_objc_object_call( obj, ___init__methodid, NULL); // init == 0xa8ba672d

   cls       = _mulle_objc_infraclass_as_class( infra);
   allocator = _mulle_objc_class_get_kvcinfo_allocator( cls);

   n_sets = 32;
   for( i = 0; i < 32; i++)
   {
      infos[ i] = _mulle_objc_kvcinfo_new( keys[ i], allocator);
      rval      = _mulle_objc_class_set_kvcinfo( cls, infos[ i]);
      if( rval == -1)
      {
         printf( "set #%d failed\n", i);
         n_sets--;
      }
   }

   n_gets = 32;
   for( i = 0; i < 32; i++)
   {
      p = _mulle_objc_class_lookup_kvcinfo( cls, keys[ i]);
      if( ! p)
      {
         n_gets--;
         continue;
      }

      if( p != infos[ i])
         printf( "get #%d failed\n", i);
   }

   if( n_sets != n_gets)
   {
      // it's normal, the cache got resized
      //  printf( "Internal inconsistency (%d sets vs. %d gets)n",  n_sets, n_gets);
   }

   _mulle_objc_class_invalidate_kvccache( cls);

   n_gets = 0;
   for( i = 0; i < 32; i++)
   {
      p = _mulle_objc_class_lookup_kvcinfo( cls, keys[ i]);
      if( p)
      {
         n_gets++;
         continue;
      }
   }

   if( n_gets)
   {
     printf( "invalidate failed\n");
   }

   // [obj release];
   mulle_objc_instance_free( obj);

   return 0;
}

