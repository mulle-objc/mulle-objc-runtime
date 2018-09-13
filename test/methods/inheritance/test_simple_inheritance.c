//
//  test_simple_inheritance.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//

#include "test_simple_inheritance.h"

#include "test_runtime_ids.h"
#include <errno.h>


#pragma mark -
#pragma mark common class setup for test


void  create_ABC_classes( struct abc_classes *classes)
{
   struct _mulle_objc_classpair   *pair;

   pair = mulle_objc_unfailingnew_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   classes->A_infra = _mulle_objc_classpair_get_infraclass( pair);
   classes->A_meta  = _mulle_objc_classpair_get_metaclass( pair);
   assert( classes->A_infra);
   assert( classes->A_meta);

   pair = mulle_objc_unfailingnew_classpair( B_classid, "B", 0, classes->A_infra);
   assert( pair);
   classes->B_infra = _mulle_objc_classpair_get_infraclass( pair);
   classes->B_meta  = _mulle_objc_classpair_get_metaclass( pair);
   assert( classes->B_infra);
   assert( classes->B_meta);

   pair = mulle_objc_unfailingnew_classpair( C_classid, "C", 0, classes->B_infra);
   assert( pair);
   classes->C_infra = _mulle_objc_classpair_get_infraclass( pair);
   classes->C_meta  = _mulle_objc_classpair_get_metaclass( pair);
   assert( classes->C_infra);
   assert( classes->C_meta);
}


void  add_ABC_classes( struct abc_classes *classes)
{
   mulle_objc_unfailingadd_infraclass( classes->A_infra);
   mulle_objc_unfailingadd_infraclass( classes->B_infra);
   mulle_objc_unfailingadd_infraclass( classes->C_infra);
}


#pragma mark -
#pragma mark specific test of inheritance w/o protocols or categories


void   *A_foo( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}

struct _mulle_objc_methodlist  A_i_list =
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
         (void *) A_foo
      }
   }
};


struct _mulle_objc_method   *A_foo_method = &A_i_list.methods[ 0];

void   *B_bar( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}


struct _mulle_objc_methodlist  B_i_list =
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
         (void *) B_bar
      }
   }
};


struct _mulle_objc_method   *B_bar_method = &B_i_list.methods[ 0];


void   *B_meta_bar( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}


struct _mulle_objc_methodlist  B_c_list =
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
         (void *) B_meta_bar
      }
   }
};


struct _mulle_objc_method   *B_meta_bar_method = &B_c_list.methods[ 0];

void   *C_bar( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   return( NULL);
}


struct _mulle_objc_methodlist  C_i_list =
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
         (void *) C_bar
      }
   }
};


struct _mulle_objc_method   *C_bar_method = &C_i_list.methods[ 0];


void   add_simple_methods( struct abc_classes  *classes)
{
   int   rval;
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_or_create_universe();

   mulle_objc_infraclass_unfailingadd_methodlist( classes->A_infra, &A_i_list);
   mulle_objc_metaclass_unfailingadd_methodlist( classes->A_meta, &universe->empty_methodlist);
   mulle_objc_infraclass_unfailingadd_methodlist( classes->B_infra, &B_i_list);
   mulle_objc_metaclass_unfailingadd_methodlist( classes->B_meta, &B_c_list);
   mulle_objc_infraclass_unfailingadd_methodlist( classes->C_infra, &C_i_list);

   // this is OK it's like adding an empty list
   mulle_objc_metaclass_unfailingadd_methodlist( classes->C_meta, NULL);

   mulle_objc_infraclass_unfailingadd_ivarlist( classes->A_infra, NULL);
   mulle_objc_infraclass_unfailingadd_ivarlist( classes->B_infra, NULL);
   mulle_objc_infraclass_unfailingadd_ivarlist( classes->C_infra, NULL);

   mulle_objc_infraclass_unfailingadd_propertylist( classes->A_infra, NULL);
   mulle_objc_infraclass_unfailingadd_propertylist( classes->B_infra, NULL);
   mulle_objc_infraclass_unfailingadd_propertylist( classes->C_infra, NULL);
}


static void   test_normal_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_infraclass_defaultsearch_method( classes->A_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   // root meta inherits from root...
   method = mulle_objc_metaclass_defaultsearch_method( classes->A_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   method = mulle_objc_infraclass_defaultsearch_method( classes->B_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes->B_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   method = mulle_objc_infraclass_defaultsearch_method( classes->C_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   method = mulle_objc_metaclass_defaultsearch_method( classes->C_meta, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);


   method = mulle_objc_infraclass_defaultsearch_method( classes->A_infra, bar_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes->A_meta, bar_methodid);
   assert( ! method);

   method = mulle_objc_infraclass_defaultsearch_method( classes->B_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_bar);

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



static void   test_inhibited_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_infraclass_defaultsearch_method( classes->A_infra, foo_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) A_foo);

   // root meta inherits from root...
   method = mulle_objc_metaclass_defaultsearch_method( classes->A_meta, foo_methodid);
   assert( ! method);

   method = mulle_objc_infraclass_defaultsearch_method( classes->B_infra, foo_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes->B_meta, foo_methodid);
   assert( ! method);

   method = mulle_objc_infraclass_defaultsearch_method( classes->C_infra, foo_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes->C_meta, foo_methodid);
   assert( ! method);


   method = mulle_objc_infraclass_defaultsearch_method( classes->A_infra, bar_methodid);
   assert( ! method);

   method = mulle_objc_metaclass_defaultsearch_method( classes->A_meta, bar_methodid);
   assert( ! method);

   method = mulle_objc_infraclass_defaultsearch_method( classes->B_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_bar);

   method = mulle_objc_metaclass_defaultsearch_method( classes->B_meta, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) B_meta_bar);

   method = mulle_objc_infraclass_defaultsearch_method( classes->C_infra, bar_methodid);
   assert( method);
   assert( _mulle_objc_method_get_implementation( method) == (mulle_objc_implementation_t) C_bar);

   method = mulle_objc_metaclass_defaultsearch_method( classes->C_meta, bar_methodid);
   assert( ! method);
}


static void   _test_simple_inheritance( struct abc_classes  *classes)
{
   create_ABC_classes( classes);
   add_simple_methods( classes);
   add_ABC_classes( classes);

   test_normal_inheritance( classes);

// now change inheritance around and see what happens
   _mulle_objc_infraclass_set_inheritance( classes->A_infra, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_infraclass_set_inheritance( classes->B_infra, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_infraclass_set_inheritance( classes->C_infra, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_metaclass_set_inheritance( classes->A_meta, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_metaclass_set_inheritance( classes->B_meta, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_metaclass_set_inheritance( classes->C_meta, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);

   test_inhibited_inheritance( classes);
}


void   test_simple_inheritance( void)
{
   struct abc_classes   classes;

   _test_simple_inheritance( &classes);
}


