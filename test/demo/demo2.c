//
//  main.m
//  test-runtime-3
//
//  Created by Nat! on 04.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
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
#include <mulle-objc-debug/mulle-objc-debug.h>
#include <stdio.h>


#define ___Object_classid      MULLE_OBJC_CLASSID( 0x58e64dae)

#define ___conforms_to_protocol__methodid   MULLE_OBJC_METHODID( 0x3d1e9472)
#define ___init__methodid      MULLE_OBJC_INIT_METHODID

//  x=B; echo "#define ___"$x"__protocolid   MULLE_OBJC_PROTOCOLID( 0x"`./build/mulle-objc-uniqueid $x`")"

#define ___E__protocolid   MULLE_OBJC_PROTOCOLID( 0x00bf080c)
#define ___D__protocolid   MULLE_OBJC_PROTOCOLID( 0x10bf213c)
#define ___A__protocolid   MULLE_OBJC_PROTOCOLID( 0x40bf6ccc)
#define ___C__protocolid   MULLE_OBJC_PROTOCOLID( 0x60bf9f2c)
#define ___B__protocolid   MULLE_OBJC_PROTOCOLID( 0x70bfb85c)


#define ___F__protocolid   MULLE_OBJC_PROTOCOLID( 0x30bf539c)
#define ___G__protocolid   MULLE_OBJC_PROTOCOLID( 0x20bf3a6c)


/* This example just checks that protocols work

   @protocol A
   @end

   @protocol B < A >
   @end

   @protocol C
   @end

   @protocol D < B, C>
   @end

   @protocol E
   @end

   @protocol F
   @end

   @interface Object < D, E >
   - (void *) init;
   - (BOOL) conformsToProtocol:(Protocol *) aProtocol
   @end

   @implementation Object
   - (void *) init
   {
      return( self);
   }
   @end
   int   main( int argc, const char * argv[])
   {
      Object  *obj;

      obj = [[Object alloc] init];
      if( [obj conformsToProtocol:@protocol( A)])
         printf( "A\n");
      if( [obj conformsToProtocol:@protocol( B)])
         printf( "B\n");
      if( [obj conformsToProtocol:@protocol( C)])
         printf( "C\n");
      if( [obj conformsToProtocol:@protocol( D)])
         printf( "D\n");
      if( [obj conformsToProtocol:@protocol( E)])
         printf( "E\n");
      if( [obj conformsToProtocol:@protocol( F)])
         printf( "F\n");
      if( [obj conformsToProtocol:@protocol( G)])
         printf( "G\n");
      [obj release];
      return 0;
   }
  */


// @interface Object

struct Object;


static void   *Object_init( struct Object *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( self);
}


static int   Object_conforms_to_protocol( struct Object *self, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_objc_protocolid_t   protocolid;

   protocolid = ((struct { mulle_objc_protocolid_t  protocolid; } *) _params)->protocolid;
   return( _mulle_objc_infraclass_conformsto_protocolid( _mulle_objc_object_get_infraclass( (void *) self),  protocolid));
}



static struct _mulle_objc_methodlist   Object_class_methodlist;

//
// for a reason I have forgotten, I couldn't use this in the regular header
// with the empty array...
//
struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods; // must be #0 and same as struct _mulle_objc_ivarlist
   void                        *owner;
   struct _mulle_objc_method   methods[];
};

static struct _gnu_mulle_objc_methodlist   Object_instance_methodlist =
{
   2,
   NULL,
   {
      {
         {
            ___conforms_to_protocol__methodid,
            "@:*i",
            "conformsToProtocol:",
            0
         },
         (mulle_objc_implementation_t) Object_conforms_to_protocol
      },
      {
         {
            ___init__methodid,
            "@:",
            "init",
            0
         },
         (mulle_objc_implementation_t) Object_init
      },
   }
};


// can't be enum, because must be void * to stay compatible with legacy
// runtimes

// enum is wrong if sizeof( int) ! sizeof( mulle_objc_protocolid_t)

struct _gnu_mulle_objc_protocollist
{
   unsigned int                  n_protocols;
   struct _mulle_objc_protocol   protocols[];
};

static struct _gnu_mulle_objc_protocollist   Object_protocollist =
{
   5,
   {
      // keep sorted by protocolid
      { ___E__protocolid, "E" },
      { ___D__protocolid, "D" },
      { ___A__protocolid, "A" },
      { ___C__protocolid, "C" },
      { ___B__protocolid, "B" },
   }
};


static struct _mulle_objc_loadclass  Object_loadclass =
{
   ___Object_classid,
   "Object",
   0,

   0,
   NULL,
   0,

   -1,
   4,

   NULL,
   &Object_class_methodlist,
   (struct _mulle_objc_methodlist *) &Object_instance_methodlist,
   NULL,
   (struct _mulle_objc_protocollist *) &Object_protocollist
};


struct _mulle_objc_loadclasslist class_list =
{
   1,
   &Object_loadclass
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


#define UNIVERSE_ID   0x7c5f7f6b
#define UNIVERSE_NAME "toy universe"

static struct _mulle_objc_loaduniverse  universe_info =
{
   UNIVERSE_ID,
   UNIVERSE_NAME
};


static struct _mulle_objc_loadinfo  load_info =
{
   {
      MULLE_OBJC_RUNTIME_LOAD_VERSION,
      MULLE_OBJC_RUNTIME_VERSION,
      0,
      0,
      TPS_BIT | FCS_BIT | TAO_BIT
   },
   &universe_info,
   &class_list
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
   fprintf( stderr, "__register_mulle_objc_universe done");
   return( universe);
}


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass    *cls;
   struct _mulle_objc_object        *obj;
   struct _mulle_objc_universe      *universe;

   // windows...
#if ! defined( __clang__) && ! defined( __GNUC__)
   __load();
#endif

   // obj = [[Object alloc] init];

   cls = mulle_objc_global_lookup_infraclass_nofail( UNIVERSE_ID, ___Object_classid);
   _mulle_objc_infraclass_setup_if_needed( cls);
   obj = mulle_objc_infraclass_alloc_instance( cls);
   obj = mulle_objc_object_call( obj, ___init__methodid, NULL);

   // if( [obj conformsToProtocol:@protocol( A)])
   //    printf( "A\n");

   universe = mulle_objc_global_get_universe( UNIVERSE_ID);
   mulle_objc_universe_dotdump_to_directory( universe, ".");

   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___A__protocolid } ))
      printf( "A\n");
   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___B__protocolid } ))
      printf( "B\n");
   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___C__protocolid } ))
      printf( "C\n");
   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___D__protocolid } ))
      printf( "D\n");
   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___E__protocolid } ))
      printf( "E\n");
   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___F__protocolid } ))
      printf( "F\n");
   if( mulle_objc_object_call( obj, ___conforms_to_protocol__methodid, &(struct { mulle_objc_protocolid_t a;  }){ .a = ___G__protocolid } ))
      printf( "G\n");

   // [obj print];

   // [obj release];
   mulle_objc_instance_free( obj);
   _mulle_objc_universe_release( universe); // since its not default

   return 0;
}

