//
//  test_category_inheritance.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle_objc_runtime/mulle_objc_runtime.h>

#include "test_runtime_ids.h"
#include "test_simple_inheritance.h"

#include <errno.h>


static void   *A_cat_foo( struct _mulle_objc_object *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}


static struct _mulle_objc_methodlist  A_cat_list =
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


static  struct _mulle_objc_methodlist  B_cat_list =
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
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes->C_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);
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
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_meta_bar);
}



void   test_category_inheritance( void)
{
   struct abc_classes           classes;
   struct _mulle_objc_method    *method;

   create_ABC_classes( &classes);
   add_simple_methods( &classes);

   add_ABC_classes( &classes);

   // add foo too A meta (!) class
   mulle_objc_infraclass_unfailingadd_methodlist( classes.A_infra, &A_cat_list);
   mulle_objc_infraclass_unfailingadd_methodlist( classes.B_infra, &B_cat_list);
   test_normal_foo_inheritance( &classes);
   test_normal_bar_inheritance( &classes);

   // now change inheritance around and see what happens
   _mulle_objc_infraclass_set_inheritance( classes.A_infra, MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES);

   method = mulle_objc_infraclass_defaultsearch_method( classes.A_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   // root meta inherits from root...
   method = mulle_objc_metaclass_defaultsearch_method( classes.A_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes.B_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes.C_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   //
   // the flag is only class specific and doesn't inherit down
   //
   _mulle_objc_infraclass_set_inheritance( classes.A_infra, 0);
   _mulle_objc_infraclass_set_inheritance( classes.C_infra, MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES);

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

   method = mulle_objc_metaclass_defaultsearch_method( classes.C_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_cat_foo);
}
