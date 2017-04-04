//
//  test_protocol_inheritance.c
//  mulle-objc-runtime
//
//  Created by Nat! on 13.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc/mulle_objc.h>

#include "test_runtime_ids.h"
#include "test_simple_inheritance.h"

#include <errno.h>
#include <stdio.h>


/* a protocol is just a number like a class ID */

/* class scheme B -> A where A has protocol C */

static void  create_ABC_PROTO_classes( struct abc_classes *classes)
{
   struct _mulle_objc_classpair   *pair;

   pair = mulle_objc_unfailing_new_classpair( C_classid, "C", 0, NULL);
   assert( pair);
   classes->C_cls      = _mulle_objc_classpair_get_infraclass( pair);
   classes->C_meta_cls = _mulle_objc_class_get_metaclass( classes->C_cls);
   assert( classes->C_meta_cls);

   // must conform to own protocol
   _mulle_objc_class_add_protocol( classes->C_cls, C_classid);
   _mulle_objc_class_add_protocol( classes->C_meta_cls, C_classid);

   pair = mulle_objc_unfailing_new_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   classes->A_cls      = _mulle_objc_classpair_get_infraclass( pair);
   classes->A_meta_cls = _mulle_objc_class_get_metaclass( classes->A_cls);
   assert( classes->A_meta_cls);
   assert( classes->A_meta_cls == _mulle_objc_classpair_get_metaclass( pair));

   _mulle_objc_class_add_protocol( classes->A_cls, C_classid);
   _mulle_objc_class_add_protocol( classes->A_meta_cls, C_classid);

   pair = mulle_objc_unfailing_new_classpair( B_classid, "B", 0, classes->A_cls);
   assert( pair);
   classes->B_cls      = _mulle_objc_classpair_get_infraclass( pair);
   assert( classes->B_cls);
   classes->B_meta_cls = _mulle_objc_class_get_metaclass( classes->B_cls);
   assert( classes->A_meta_cls);

}


static void  add_ABC_PROTO_classes( struct abc_classes *classes)
{
   mulle_objc_unfailing_add_class( classes->A_cls);
   mulle_objc_unfailing_add_class( classes->B_cls);
   mulle_objc_unfailing_add_class( classes->C_cls);
}



static void   *A_cat_foo( struct _mulle_objc_object *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}



struct _mulle_objc_methodlist  A_cat_list =
{
   1,
   NULL,
   {
      {
         {
            foo_methodid,
            "foo",
            "@:",
            0
         },
         (void *) A_cat_foo
      }
   }
};

static struct _mulle_objc_method   *A_cat_foo_method = &A_cat_list.methods[ 0];


static void   *B_cat_bar( struct _mulle_objc_object *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}




struct _mulle_objc_methodlist  B_cat_list =
{
   1,
   NULL,
   {
      {
         {
            bar_methodid,
            "bar",
            "@:",
            0
         },
         (void *) B_cat_bar
      }
   }
};

static struct _mulle_objc_method   *B_cat_bar_method = &B_cat_list.methods[ 0];



static void   test_normal_foo_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_class_search_method( classes->A_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   // root meta inherits from root...
   method = mulle_objc_class_search_method( classes->A_meta_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   method = mulle_objc_class_search_method( classes->B_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   method = mulle_objc_class_search_method( classes->B_meta_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   method = mulle_objc_class_search_method( classes->C_cls, foo_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->C_meta_cls, foo_methodid);
   assert( ! method);
}


static void   test_normal_bar_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_class_search_method( classes->A_cls, bar_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->A_meta_cls, bar_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->B_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) B_cat_bar);

   method = mulle_objc_class_search_method( classes->B_meta_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) B_meta_bar);

   method = mulle_objc_class_search_method( classes->C_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) C_bar);

   method = mulle_objc_class_search_method( classes->C_meta_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) C_bar);
}


#if DEBUG
static int   print_method( struct _mulle_objc_method *method, struct _mulle_objc_class *cls, void *userinfo)
{
   printf( "\t%s %s : %c%s(%p)\n", _mulle_objc_class_get_name( cls), _mulle_objc_class_is_metaclass( userinfo) ? "META" : "    ", _mulle_objc_class_is_metaclass( userinfo) ? '+' : '-', _mulle_objc_method_get_name( method), _mulle_objc_method_get_implementation( method));
   return( 0);
}
#endif


void   test_protocol_inheritance( void)
{
   struct abc_classes           classes;
   struct _mulle_objc_method    *method;

   create_ABC_PROTO_classes( &classes);
   add_simple_methods( &classes);

   add_ABC_PROTO_classes( &classes);

   // add foo too A meta (!) class
   mulle_objc_class_add_methodlist( classes.A_cls, &A_cat_list);
   mulle_objc_class_add_methodlist( classes.B_cls, &B_cat_list);

   test_normal_foo_inheritance( &classes);
   test_normal_bar_inheritance( &classes);

   // now change inheritance around and see what happens
   _mulle_objc_class_set_inheritance( classes.A_cls, MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES);

   method = mulle_objc_class_search_method( classes.A_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   // root meta inherits from root...
   method = mulle_objc_class_search_method( classes.A_meta_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   method = mulle_objc_class_search_method( classes.B_meta_cls, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) A_cat_foo);

   method = mulle_objc_class_search_method( classes.C_cls, foo_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes.C_meta_cls, foo_methodid);
   assert( ! method);

   //
   // the flag is only class specific and doesn't inherit down
   //
   method = mulle_objc_class_search_method( classes.A_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) C_bar);

   method = mulle_objc_class_search_method( classes.A_meta_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) C_bar);

   method = mulle_objc_class_search_method( classes.B_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) B_cat_bar);

   method = mulle_objc_class_search_method( classes.B_meta_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) B_meta_bar);

   method = mulle_objc_class_search_method( classes.C_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) C_bar);

   method = mulle_objc_class_search_method( classes.C_meta_cls, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_methodimplementation_t) C_bar);

#if DEBUG
   {
      printf( "B_cls:\n");
      _mulle_objc_class_walk_methods( classes.B_cls, MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS, print_method, classes.B_cls);
      printf( "\nB_meta_cls:\n");
      _mulle_objc_class_walk_methods( classes.B_meta_cls, MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS, print_method, classes.B_meta_cls);
   }
#endif
}
