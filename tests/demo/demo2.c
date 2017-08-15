//
//  main.m
//  test-runtime-3
//
//  Created by Nat! on 04.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__   1
#define __MULLE_OBJC_NO_TRT__   1

#include <mulle_objc_runtime/mulle_objc_runtime.h>

#include <stdio.h>


#define ___Foo_classid         MULLE_OBJC_CLASSID( 0x40413ff3)
#define ___Object_classid      MULLE_OBJC_CLASSID( 0x5bd95814)

#define ___conforms_to_protocol__methodid   MULLE_OBJC_METHODID( 0x10691aa7)
#define ___setA_b___methodid   MULLE_OBJC_METHODID( 0x3c146ada)
#define ___print__methodid     MULLE_OBJC_METHODID( 0x4bb743c2)
#define ___init__methodid      MULLE_OBJC_METHODID( 0x50c63a23)

#define ___b___ivarid          MULLE_OBJC_IVARID( 0x050c5d7d)
#define ___a___ivarid          MULLE_OBJC_IVARID( 0x050c5d7e)

#define ___G__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d58)
#define ___F__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d59)
#define ___E__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d5a)
#define ___D__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d5b)
#define ___C__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d5c)
#define ___B__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d5d)
#define ___A__protocolid  MULLE_OBJC_PROTOCOLID( 0x050c5d5e)


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
            "conformsToProtocol:",
            "@:*i",
            0
         },
         (mulle_objc_methodimplementation_t) Object_conforms_to_protocol
      },
      {
         {
            ___init__methodid,
            "init",
            "@:",
            0
         },
         (mulle_objc_methodimplementation_t) Object_init
      }
   }
};


// can't be enum, because must be void * to stay compatible with legacy
// runtimes

// enum is wrong if sizeof( int) ! sizeof( mulle_objc_protocolid_t)

struct _gnu_mulle_objc_protcollist
{
   unsigned int                  n_protocols;
   struct _mulle_objc_protocol   protocols[];
};

static struct _gnu_mulle_objc_protcollist   Object_protocollist =
{
   5,
   {
      { ___A__protocolid, "A" },
      { ___B__protocolid, "B" },
      { ___C__protocolid, "C" },
      { ___D__protocolid, "D" },
      { ___E__protocolid, "E" }
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
   (struct _mulle_objc_protcollist *) &Object_protocollist
};


struct _mulle_objc_loadclasslist class_list =
{
   1,
   &Object_loadclass
};


static struct _mulle_objc_loadinfo  load_info =
{
   {
      MULLE_OBJC_RUNTIME_LOAD_VERSION,
      MULLE_OBJC_RUNTIME_VERSION,
      0,
      0,
      0
   },
   &class_list
};


#if defined( __clang__) || defined( __GNUC__)
__attribute__((constructor))
#endif
static void  __load()
{
   static int  has_loaded;

   fprintf( stderr, "--> __load\n");

   // windows w/o mulle-clang
   if( has_loaded)
      return;
   has_loaded = 1;

   mulle_objc_loadinfo_unfailingenqueue( &load_info);
}


struct _mulle_objc_universe  *__get_or_create_mulle_objc_universe( void)
{
   struct _mulle_objc_universe    *universe;

   universe = __mulle_objc_get_universe();
   if( ! _mulle_objc_universe_is_initialized( universe))
   {
      _mulle_objc_universe_bang( universe, 0, 0, NULL);
      universe->config.ignore_ivarhash_mismatch = 1;
   }
   return( universe);
}


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass    *cls;
   struct _mulle_objc_object        *obj;

   // windows...
#if ! defined( __clang__) && ! defined( __GNUC__)
   __load();
#endif

   // obj = [[Object alloc] init];

   cls = mulle_objc_unfailingfastlookup_infraclass( ___Object_classid);
   obj = mulle_objc_infraclass_alloc_instance( cls, NULL);
   obj = mulle_objc_object_call( obj, ___init__methodid, NULL);

   // if( [obj conformsToProtocol:@protocol( A)])
   //    printf( "A\n");

   mulle_objc_dotdump_to_tmp();

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
   mulle_objc_object_free( obj, NULL);

   return 0;
}

