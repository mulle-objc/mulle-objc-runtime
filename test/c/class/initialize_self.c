//
//  initialize_self.c
//  mulle-objc-runtime
//
//  Tests that +initializeSelf is called during class setup and
//  +deinitializeSelf during teardown, and that the metaclass
//  classpropertylock is initialized/destroyed around them.
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

#define MULLE_OBJC_DEFINE_REGISTER_UNIVERSE

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


// mulle-objc-uniqueid Foo -> 0xc7e16770
#define ___Foo_classid   MULLE_OBJC_CLASSID( 0xc7e16770)

static int   initialize_self_called;
static int   deinitialize_self_called;


static void   *Foo_initializeSelf( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   initialize_self_called++;
   printf( "+initializeSelf\n");
   return( NULL);
}


static void   *Foo_deinitializeSelf( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   deinitialize_self_called++;
   printf( "+deinitializeSelf\n");
   return( NULL);
}


struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods;
   void                        *owner;
   struct _mulle_objc_method   methods[];
};


static struct _gnu_mulle_objc_methodlist  Foo_initializeself_methodlist =
{
   .n_methods = 1,
   .methods   =
   {
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_INITIALIZESELF_METHODID,
            .signature = "v@:",
            .name      = "initializeSelf",
         },
         .value = (mulle_objc_implementation_t) Foo_initializeSelf
      }
   }
};


static struct _gnu_mulle_objc_methodlist  Foo_deinitializeself_methodlist =
{
   .n_methods = 1,
   .methods   =
   {
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_DEINITIALIZESELF_METHODID,
            .signature = "v@:",
            .name      = "deinitializeSelf",
         },
         .value = (mulle_objc_implementation_t) Foo_deinitializeSelf
      }
   }
};


static struct _gnu_mulle_objc_methodlist  Foo_class_methodlist =
{
   .n_methods = 2,
   .methods   =
   {
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_INITIALIZESELF_METHODID,
            .signature = "v@:",
            .name      = "initializeSelf",
         },
         .value = (mulle_objc_implementation_t) Foo_initializeSelf
      },
      {
         .descriptor =
         {
            .methodid  = MULLE_OBJC_DEINITIALIZESELF_METHODID,
            .signature = "v@:",
            .name      = "deinitializeSelf",
         },
         .value = (mulle_objc_implementation_t) Foo_deinitializeSelf
      }
   }
};


static struct _mulle_objc_loadclass  Foo_loadclass =
{
   .base.classid         = ___Foo_classid,
   .base.classname       = "Foo",
   .base.classmethods    = (struct _mulle_objc_methodlist *) &Foo_class_methodlist,

   .fastclassindex       = -1,
   .instancesize         = 0,
};


struct _gnu_mulle_objc_loadclasslist
{
   unsigned int                    n_loadclasses;
   struct _mulle_objc_loadclass    *loadclasses[];
};


static struct _gnu_mulle_objc_loadclasslist  class_list =
{
   .n_loadclasses = 1,
   .loadclasses   = { &Foo_loadclass }
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
   .version =
   {
      .load    = MULLE_OBJC_RUNTIME_LOAD_VERSION,
      .runtime = MULLE_OBJC_RUNTIME_VERSION,
      .bits    = TPS_BIT | FCS_BIT | TAO_BIT
   },
   .loadclasslist = (struct _mulle_objc_loadclasslist *) &class_list,
};


MULLE_C_CONSTRUCTOR( __load)
static void  __load()
{
   static int  has_loaded;

   if( has_loaded)
      return;
   has_loaded = 1;

   mulle_objc_loadinfo_enqueue_nofail( &load_info);
}


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
      universe->config.pedantic_exit = 1;
   }
   return( universe);
}


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;

#if ! defined( __clang__) && ! defined( __GNUC__)
   __load();
#endif

   // trigger class setup, which should call +initializeSelf
   infra = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID, ___Foo_classid);
   _mulle_objc_infraclass_setup_if_needed( infra);

   if( ! initialize_self_called)
   {
      printf( "FAIL: +initializeSelf not called\n");
      return( 1);
   }

   // verify lock was initialized (depth != -1 sentinel)
   meta = _mulle_objc_infraclass_get_metaclass( infra);
   if( _mulle_atomic_pointer_read( &meta->classpropertylock._depth) == (void *) -1)
   {
      printf( "FAIL: classpropertylock not initialized\n");
      return( 1);
   }

   // teardown universe, which should call +deinitializeSelf
   _mulle_objc_universe_release( mulle_objc_global_get_universe( MULLE_OBJC_DEFAULTUNIVERSEID));

   if( ! deinitialize_self_called)
   {
      printf( "FAIL: +deinitializeSelf not called\n");
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
