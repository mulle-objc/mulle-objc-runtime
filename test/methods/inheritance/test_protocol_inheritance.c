//
//  test_protocol_inheritance.c
//  mulle-objc-runtime
//
//  Created by Nat! on 13.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include "test_runtime_ids.h"
#include "test_simple_inheritance.h"

#include <errno.h>
#include <stdio.h>


/* a protocol is just a number like a class ID */

/* class scheme B -> A where B has protocol C */

static void  create_ABC_PROTO_classes( struct abc_classes *classes)
{
   struct _mulle_objc_classpair       *pair;
   struct _mulle_objc_protocollist    *protocollist;
   struct _mulle_objc_runtime         *runtime;

   pair = mulle_objc_unfailingnew_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   classes->A_infra = _mulle_objc_classpair_get_infraclass( pair);
   classes->A_meta  = _mulle_objc_classpair_get_metaclass( pair);
   assert( classes->A_meta);
   assert( classes->A_meta == _mulle_objc_class_get_metaclass( _mulle_objc_infraclass_as_class( classes->A_infra)));

   pair = mulle_objc_unfailingnew_classpair( B_classid, "B", 0, classes->A_infra);
   assert( pair);
   classes->B_infra = _mulle_objc_classpair_get_infraclass( pair);
   assert( classes->B_infra);
   classes->B_meta  = _mulle_objc_classpair_get_metaclass( pair);
   assert( classes->A_meta);

   pair = mulle_objc_unfailingnew_classpair( C_classid, "C", 0, NULL);
   assert( pair);
   classes->C_infra = _mulle_objc_classpair_get_infraclass( pair);
   classes->C_meta  = _mulle_objc_classpair_get_metaclass( pair);
   assert( classes->C_meta);

   // must conform to own protocol
   protocollist  = mulle_objc_alloc_protocollist( 1);
   protocollist->n_protocols = 1;
   protocollist->protocols[ 0].protocolid = C_classid;
   protocollist->protocols[ 0].name       = "C";

   mulle_objc_classpair_unfailingadd_protocollist( pair, protocollist);
}


static void  add_ABC_PROTO_classes( struct abc_classes *classes)
{
   static mulle_objc_protocolid_t     classprotocolids[] = { C_classid, MULLE_OBJC_NO_PROTOCOLID };
   struct _mulle_objc_classpair       *pair;
   struct _mulle_objc_protocollist    *protocollist;

   mulle_objc_unfailingadd_infraclass( classes->A_infra);
   mulle_objc_unfailingadd_infraclass( classes->B_infra);
   mulle_objc_unfailingadd_infraclass( classes->C_infra);

   pair = _mulle_objc_infraclass_get_classpair( classes->B_infra);

   protocollist  = mulle_objc_alloc_protocollist( 1);
   protocollist->n_protocols = 1;
   protocollist->protocols[ 0].protocolid = C_classid;
   protocollist->protocols[ 0].name       = "C";

   mulle_objc_classpair_unfailingadd_protocollist( pair, protocollist);
   mulle_objc_classpair_unfailingadd_protocolclassids( pair, classprotocolids);
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
            "@:",
            "foo",
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
            "@:",
            "bar",
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

   method = mulle_objc_infraclass_defaultsearch_method( classes->A_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   // root meta inherits from root...
   method = mulle_objc_metaclass_defaultsearch_method( classes->A_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   method = mulle_objc_infraclass_defaultsearch_method( classes->B_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes->B_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   method = mulle_objc_infraclass_defaultsearch_method( classes->C_infra, foo_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes->C_meta, foo_methodid);
   assert( ! method);
}


static void   test_normal_bar_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_infraclass_defaultsearch_method( classes->A_infra, bar_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes->A_meta, bar_methodid);
   assert( ! method);

   method = mulle_objc_infraclass_defaultsearch_method( classes->B_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_cat_bar);

   method = mulle_objc_metaclass_defaultsearch_method( classes->B_meta, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_meta_bar);

   method = mulle_objc_infraclass_defaultsearch_method( classes->C_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) C_bar);

   method = mulle_objc_metaclass_defaultsearch_method( classes->C_meta, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) C_bar);
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
   mulle_objc_infraclass_unfailingadd_methodlist( classes.A_infra, &A_cat_list);
   mulle_objc_infraclass_unfailingadd_methodlist( classes.B_infra, &B_cat_list);

   test_normal_foo_inheritance( &classes);
   test_normal_bar_inheritance( &classes);

   // now change inheritance around and see what happens
   _mulle_objc_infraclass_set_inheritance( classes.A_infra, MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES);

   method = mulle_objc_infraclass_defaultsearch_method( classes.A_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   // root meta inherits from root...
   method = mulle_objc_metaclass_defaultsearch_method( classes.A_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes.B_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   method = mulle_objc_infraclass_defaultsearch_method( classes.C_infra, foo_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes.C_meta, foo_methodid);
   assert( ! method);

   //
   // the flag is only class specific and doesn't inherit down
   //
   method = mulle_objc_infraclass_defaultsearch_method( classes.A_infra, bar_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes.A_meta, bar_methodid);
   assert( ! method);

   method = mulle_objc_infraclass_defaultsearch_method( classes.B_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_cat_bar);

   method = mulle_objc_metaclass_defaultsearch_method( classes.B_meta, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_meta_bar);

   method = mulle_objc_infraclass_defaultsearch_method( classes.C_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) C_bar);

   method = mulle_objc_metaclass_defaultsearch_method( classes.C_meta, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) C_bar);

#if DEBUG
   {
      printf( "B_infra:\n");
      _mulle_objc_class_walk_methods( classes.B_infra, MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS, print_method, classes.B_infra);
      printf( "\nB_meta:\n");
      _mulle_objc_class_walk_methods( classes.B_meta, MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS, print_method, classes.B_meta);
   }
#endif
}
