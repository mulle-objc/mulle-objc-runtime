//
//  main.c
//  test-runtime-2
//
//  Created by Nat! on 19/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__  1
#define __MULLE_OBJC_NO_TRT__  1

#include <mulle_objc_runtime/mulle_objc_runtime.h>

#include <stdio.h>


/* in this example, Foo inherits from Object and has a category Foo( Print)
   what follows is the construction of this code in C without a compiler

   @interface Object
      - (void *) init;
   @end

   @implementation Object
   - (void *) init
   {
      return( self);
   }
   @end

   @interface Foo : Object
   {
      int  a;
      int  b;
   }
   - (void) setA:(int) a b:(int) b;
   @end

   @implementation Foo
   - (void *) init
   {
      [super init];
      self->a = 1;
      self->b = 2;
      return( self);
   }
   - (void) setA:(int) a b:(int) b
   {
      self->a = a;
      self->b = b;
   }
   @end

   @interface Foo ( Print)
   - (void) print;
   @end
   @implementation Foo ( Print)
   - (void) print
   {
      printf( "%d%d\n", self->a, self->b);
   }
   @end

   int   main( int argc, const char * argv[])
   {
      Foo  *obj;

      obj = [[Foo alloc] init];
      [obj setA:18 b:48];
      [obj print];
      [obj release];
      return 0;
   }
  */


// generate these ids with mulle-objc-uniqueid
// ex. mulle-objc-uniqueid Foo ->  40413ff3

#define ___Foo_classid         MULLE_OBJC_CLASSID( 0x40413ff3)
#define ___Object_classid      MULLE_OBJC_CLASSID( 0x5bd95814)
#define ___Print_categoryid    MULLE_OBJC_CATEGORYID( 0xcbe38fa2)

#define ___setA_b___methodid   MULLE_OBJC_METHODID( 0x3c146ada)
#define ___print__methodid     MULLE_OBJC_METHODID( 0x4bb743c2)
#define ___init__methodid      MULLE_OBJC_METHODID( 0x50c63a23)

#define ___b___ivarid          MULLE_OBJC_IVARID( 0x050c5d7d)
#define ___a___ivarid          MULLE_OBJC_IVARID( 0x050c5d7e)


//
// use gnu style initalization (empty methods[], because it's less painful)
//
struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods;
   void                        *owner;
   struct _mulle_objc_method   methods[];
};


struct _gnu_mulle_objc_ivarlist
{
   unsigned int              n_ivars;

   struct _mulle_objc_ivar   ivars[];
};


struct _gnu_mulle_objc_loadclasslist
{
   unsigned int                    n_loadclasses;
   struct _mulle_objc_loadclass    *loadclasses[];
};


struct _gnu_mulle_objc_superlist
{
   unsigned int                n_supers;
   struct _mulle_objc_super    supers[];
};





// @interface Object
//    - (void *) init;
// @end
struct Object;


// @implementation Object
// - (void *) init
// {
//    return( self);
// }
// @end


static void   *Object_init( struct Object *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( self);
}


// this is how a methodlist is defined:
//
// struct _mulle_objc_methodlist
// {
//    unsigned int                 n_methods;
//    char                         *owner;
//    struct _mulle_objc_method    methods[ 1];
// };
//
// this is how a method is defined:
//
// struct _mulle_objc_method
// {
//   struct _mulle_objc_methoddescriptor  descriptor;
//   mulle_objc_methodimplementation_t    implementation;
// };
//
// this is how a method descriptor is defined:
//
// struct _mulle_objc_methoddescriptor
// {
//    mulle_objc_methodid_t   methodid;
//    char                    *name;
//    char                    *signature;
//    unsigned int            bits;
// };


static struct _mulle_objc_methodlist  Object_instance_methodlist =
{
   1,				// n_methods
   NULL,			// owner, for debugging
   {
      {
         {
            ___init__methodid,	// descriptor.methodid
            "init",		         // descriptor.name
            "@:",		            // descriptor.signature
            0			            // descriptor.bits
         },
         (mulle_objc_methodimplementation_t) Object_init	// implementation
      }
   }
};



// this is how a loadclass looks like:
//
// struct _mulle_objc_loadclass
// {
//    mulle_objc_classid_t              classid;
//    char                              *class_name;
//    mulle_objc_hash_t                 class_ivarhash;
//
//    mulle_objc_classid_t              superclass_uniqueid;
//    char                              *superclass_name;
//    mulle_objc_hash_t                 superclass_ivarhash;
//
//    int                               fastclassindex;
//    int                               instance_size;
//
//    struct _mulle_objc_ivarlist       *instance_variables;
//
//    struct _mulle_objc_methodlist     *class_methods;
//    struct _mulle_objc_methodlist     *instance_methods;
//    struct _mulle_objc_propertylist   *properties;
//
//    mulle_objc_protocolid_t           *protocol_uniqueids;
// };


static struct _mulle_objc_loadclass  Object_loadclass =
{
   ___Object_classid,
   "Object",
   0,

   0,
   NULL,
   0,

   -1,
   4,  // some size

   NULL,
   NULL,
   &Object_instance_methodlist,
   NULL,

   NULL
};


// @interface Foo : Object

#define __Foo_iVARs  \
   int   a;          \
   int   b

struct Foo
{
   __Foo_iVARs;
};

// @end

// @implementation Foo
//
//- (void *) init
//{
//   self = [super init];
//
//   a = 1;
//   b = 2;
//
//   return( self);
//}
//

static void   *Foo_init( struct Foo *self, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_objc_superid_t   superid;

   superid = mulle_objc_superid_from_classid_and_methodid( ___Foo_classid, ___init__methodid);

   self = (void *) mulle_objc_object_call_superid( (void *) self, _cmd, _params, superid);

   self->a = 1;
   self->b = 2;

   return( self);
}


// - (void) setA:(int) a b:(int) b
// {
//    self->a = a;
//    self->b = b;
// }
static void   Foo_setA_b_( struct Foo *self, mulle_objc_methodid_t _cmd, struct { int _a; int _b; } *_params)
{
   int   a = _params->_a;
   int   b = _params->_b;

   self->a = a;
   self->b = b;
}

// @end


static struct _gnu_mulle_objc_ivarlist  Foo_ivarlist =
{
   2,
   // must be sorted by ivarid !!!
   {
      {
         {
            ___b___ivarid,
            "b",
            "i"
         },
         offsetof( struct Foo, b)
      },
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
   2,
   NULL,
   {
      {
         {
            ___setA_b___methodid,
            "setA:b:",
            "@:ii",
            0
         },
         (mulle_objc_methodimplementation_t) Foo_setA_b_
      },
      {
         {
            ___init__methodid,
            "init",
            "@:",
            0
         },
         (mulle_objc_methodimplementation_t) Foo_init
      }
   }
};


static struct _mulle_objc_loadclass  Foo_loadclass =
{
   ___Foo_classid,
   "Foo",
   0,

   ___Object_classid,
   "Object",
   0,

   -1,
   sizeof( struct Foo),

   (struct _mulle_objc_ivarlist *)  &Foo_ivarlist,
   NULL,
   (struct _mulle_objc_methodlist *) &Foo_instance_methodlist,
   NULL,

   NULL
};


// @implementation Foo ( Print)
//

// - (void *) print
// {
//    printf( "%d%d\n", a, b);
// }
static void   Foo_Print_print( struct Foo *self, mulle_objc_methodid_t _cmd, void *_params)
{
   printf( "%d%d\n", self->a, self->b);
}

// @end


static struct _mulle_objc_methodlist  Foo_Print_instance_methodlist =
{
   1,
   NULL,
   {
      {
         {
            ___print__methodid,
            "print",
            "@:",
            0
         },
         (mulle_objc_methodimplementation_t) Foo_Print_print
      }
   }
};


// struct _mulle_objc_loadcategory
// {
//    char                              *category_name;
//
//    mulle_objc_classid_t              classid;
//    char                              *class_name;         // useful ??
//    mulle_objc_hash_t                 class_ivarhash;
//
//    struct _mulle_objc_methodlist     *class_methods;
//    struct _mulle_objc_methodlist     *instance_methods;
//    struct _mulle_objc_propertylist   *properties;
//
//    mulle_objc_protocolid_t           *protocol_uniqueids;
// };

struct _mulle_objc_loadcategory   Foo_Print_category_load =
{
   ___Print_categoryid,
   "Print",

   ___Foo_classid,
   "Foo",
   0,

   NULL,
   &Foo_Print_instance_methodlist,
   NULL,

   NULL
};


struct _mulle_objc_loadcategorylist category_list =
{
   1,
   &Foo_Print_category_load
};



struct _gnu_mulle_objc_loadclasslist  class_list =
{
   2,
   {
      &Object_loadclass,
      &Foo_loadclass
   }
};


#define X_MULLE_OBJC_SUPERID_MAKE( c, m) \
   ((mulle_objc_superid_t) (((mulle_objc_uniqueid_t) (c) * (mulle_objc_uniqueid_t) 0x01000193) ^ (mulle_objc_uniqueid_t)(m)))


struct _gnu_mulle_objc_superlist  super_list =
{
   1,
   {
      {
         X_MULLE_OBJC_SUPERID_MAKE( ___Foo_classid, ___init__methodid),
         ___Foo_classid,
         ___init__methodid
      }
   }
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
   (struct _mulle_objc_loadclasslist *) &class_list,  // let runtime sort for us
   &category_list,
   &super_list
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

   fprintf( stderr, "--> __get_or_create_mulle_objc_universe\n");
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

   fprintf( stderr, "--> main\n");

   // obj = [[Foo alloc] init];

   fprintf( stderr, "-==> mulle_objc_unfailingfastlookup_infraclass()\n");
   cls = mulle_objc_unfailingfastlookup_infraclass( ___Foo_classid);

   fprintf( stderr, "-==> mulle_objc_infraclass_alloc_instance()\n");
   obj = mulle_objc_infraclass_alloc_instance( cls, NULL);

   fprintf( stderr, "-==> mulle_objc_object_call( ... ___init__methodid ...)\n");
   obj = (void *) mulle_objc_object_call( obj, ___init__methodid, NULL); // init == 0xa8ba672d

   // [obj setA:18 b:48];
   fprintf( stderr, "-==> mulle_objc_object_call( ... ___setA_b___methodid ...)\n");
   mulle_objc_object_call( obj, ___setA_b___methodid, &(struct { int a; int b; }){ .a = 18, .b = 48 });

   // [obj print];
   fprintf( stderr, "-==> mulle_objc_object_call( ... ___print__methodid ...)\n");
   mulle_objc_object_call( obj, ___print__methodid, NULL);

   // [obj release];
   fprintf( stderr, "-==> mulle_objc_object_free( ... )\n");
   mulle_objc_object_free( obj, NULL);

   return 0;
}


