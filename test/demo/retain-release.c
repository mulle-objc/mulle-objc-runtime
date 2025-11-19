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



// @interface Object

struct Object;


// @implementation Object


static void *   Object_alloc( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   struct _mulle_objc_infraclass  *infra = self;
   struct mulle_allocator         *allocator;
   void                           *obj;

   mulle_printf( "+alloc\n");
   allocator = _mulle_objc_infraclass_get_allocator( infra);
   obj       = _mulle_objc_infraclass_allocator_alloc_instance_extra( infra,
                                                                      0,
                                                                      allocator);
   return( obj);
}


static void *   Object_retain( void *obj, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_printf( "-retain\n");
   _mulle_objc_object_retain_inline( obj);
   return( obj);
}


static void    Object_release( void *obj, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_printf( "-release\n");
   _mulle_objc_object_release_inline( obj);
}


static void   Object_finalize( void *obj, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_printf( "-finalize\n");
}


static void   Object_dealloc( void *obj, mulle_objc_methodid_t _cmd, void *_params)
{
   mulle_printf( "-dealloc\n");
   _mulle_objc_instance_free( obj);
}



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


//
// struct _mulle_objc_descriptor
// {
//    mulle_objc_methodid_t   methodid;   // alignment to signature unclear!
//    char                    *signature; // mulle_objc_compat: signature < name
//    char                    *name;
//    uint32_t                bits;       // TODO: make this a mulle_atomic_pointer_t
// };
//
// struct _mulle_objc_method
// {
//    struct _mulle_objc_descriptor   descriptor;
//    union
//    {
//       mulle_objc_implementation_t      value;
//       mulle_atomic_functionpointer_t   implementation;
//    };
// };
//
// MEMO: should do method[].descriptor.bits properly, but skimping on this
//       because: lazy
static struct _gnu_mulle_objc_methodlist   Object_class_methodlist =
{
   .n_methods = 1,
   .methods   =
   {
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_ALLOC_METHODID,
            .signature = "@@:",
            .name      = "alloc"
         },
         .value = (mulle_objc_implementation_t) Object_alloc
      }
   }
};


static struct _gnu_mulle_objc_methodlist   Object_instance_methodlist =
{
   .n_methods = 4,
   .methods   =
   {
      // usually the methods need to be sorted by methodid (signed)
      // but we set a special bit _mulle_objc_loadinfo_unsorted as
      // we are lazy, and let the runtime do the sorting
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_RELEASE_METHODID,
            .signature = "^v@:",
            .name      = "release"
         },
         .value = (mulle_objc_implementation_t) Object_release
      },
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_RETAIN_METHODID,
            .signature = "@@:",
            .name      = "retain"
         },
         .value = (mulle_objc_implementation_t) Object_retain
      },
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_FINALIZE_METHODID,
            .signature = "^v@:",
            .name      = "finalize"
         },
         .value = (mulle_objc_implementation_t) Object_finalize
      },
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_DEALLOC_METHODID,
            .signature = "^v@:",
            .name      = "dealloc"
         },
         .value = (mulle_objc_implementation_t) Object_dealloc
      },
   }
};


static struct _mulle_objc_loadclass  Object_loadclass =
{
   .classid         = ___Object_classid,
   .classname       = "Object",

   .fastclassindex  = -1,
   .instancesize    = 0,

   .classmethods    = (struct _mulle_objc_methodlist *) &Object_class_methodlist,
   .instancemethods = (struct _mulle_objc_methodlist *) &Object_instance_methodlist,
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


// struct _mulle_objc_loadinfo
// {
//    struct mulle_objc_loadversion             version;
//
//    struct _mulle_objc_loaduniverse           *loaduniverse;
//    struct _mulle_objc_loadclasslist          *loadclasslist;
//    struct _mulle_objc_loadcategorylist       *loadcategorylist;
//    struct _mulle_objc_superlist              *loadsuperlist;
//    struct _mulle_objc_loadstringlist         *loadstringlist;
//    struct _mulle_objc_loadhashedstringlist   *loadhashedstringlist;  // optional for debugging
//
//    char   *origin;
// };
//
// struct mulle_objc_loadversion
// {
//    uint32_t   load;
//    uint32_t   runtime;
//    uint32_t   foundation;
//    uint32_t   user;
//    uint32_t   bits;
// };

static struct _mulle_objc_loadinfo  load_info =
{
   .version =
   {
      .load    = MULLE_OBJC_RUNTIME_LOAD_VERSION,
      .runtime = MULLE_OBJC_RUNTIME_VERSION,
      .bits    = TPS_BIT | FCS_BIT | TAO_BIT | _mulle_objc_loadinfo_unsorted
   },
   .loadclasslist = &class_list
};


MULLE_C_CONSTRUCTOR( __load)
static void  __load()
{
   static int  has_loaded;

   mulle_fprintf( stderr, "--> __load\n");

   // windows w/o mulle-clang
   if( has_loaded)
      return;
   has_loaded = 1;

   mulle_objc_loadinfo_enqueue_nofail( &load_info);
}


MULLE_C_EXTERN_GLOBAL
MULLE_C_CONST_RETURN 
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
   mulle_fprintf( stderr, "__register_mulle_objc_universe done\n");
   return( universe);
}


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_object       *obj;
   struct _mulle_objc_universe     *universe;

   // windows...
#if ! defined( __clang__) && ! defined( __GNUC__)
   __load();
#endif

   // obj = [Object new];

   infra = mulle_objc_global_lookup_infraclass_nofail( 0, ___Object_classid);
   _mulle_objc_infraclass_setup_if_needed( infra);

   obj = mulle_objc_object_call( infra, MULLE_OBJC_ALLOC_METHODID, infra);

   // obj = [obj retain];
   obj = mulle_objc_object_call( obj, MULLE_OBJC_RETAIN_METHODID, obj);

   // [obj release];
   mulle_objc_object_call( obj, MULLE_OBJC_RELEASE_METHODID, obj);

   // [obj release];
   mulle_objc_object_call( obj, MULLE_OBJC_RELEASE_METHODID, obj);

   return 0;
}

