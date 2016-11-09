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

   pair = mulle_objc_unfailing_new_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   classes->A_cls      = _mulle_objc_classpair_get_infraclass( pair);
   classes->A_meta_cls = _mulle_objc_class_get_metaclass( classes->A_cls);
   assert( classes->A_cls);
   assert( classes->A_meta_cls);

   pair = mulle_objc_unfailing_new_classpair( B_classid, "B", 0, classes->A_cls);
   assert( pair);
   classes->B_cls      = _mulle_objc_classpair_get_infraclass( pair);
   classes->B_meta_cls = _mulle_objc_class_get_metaclass( classes->B_cls);
   assert( classes->B_cls);
   assert( classes->B_meta_cls);

   pair = mulle_objc_unfailing_new_classpair( C_classid, "C", 0, classes->B_cls);
   assert( pair);
   classes->C_cls      = _mulle_objc_classpair_get_infraclass( pair);
   classes->C_meta_cls = _mulle_objc_class_get_metaclass( classes->C_cls);
   assert( classes->C_cls);
   assert( classes->C_meta_cls);
}


void  add_ABC_classes( struct abc_classes *classes)
{
   mulle_objc_unfailing_add_class( classes->A_cls);
   mulle_objc_unfailing_add_class( classes->B_cls);
   mulle_objc_unfailing_add_class( classes->C_cls);
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
            "foo",
            "@:",
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
            "bar",
            "@:",
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
            "bar",
            "@:",
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
            "bar",
            "@:",
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
   struct _mulle_objc_runtime   *runtime;

   runtime = __get_or_create_objc_runtime();

   rval = mulle_objc_class_add_methodlist( classes->A_cls, &A_i_list);
   assert( ! rval);
   rval = mulle_objc_class_add_methodlist( classes->A_meta_cls, &runtime->empty_methodlist);
   assert( ! rval);
   rval = mulle_objc_class_add_methodlist( classes->B_cls, &B_i_list);
   assert( ! rval);
   rval = mulle_objc_class_add_methodlist( classes->B_meta_cls, &B_c_list);
   assert( ! rval);
   rval = mulle_objc_class_add_methodlist( classes->C_cls, &C_i_list);
   assert( ! rval);

   // this is OK it's like adding an empty list
   rval = mulle_objc_class_add_methodlist( classes->C_meta_cls, NULL);
   assert( ! rval);

   rval = mulle_objc_class_add_ivarlist( classes->A_cls, NULL);
   assert( ! rval);
   rval = mulle_objc_class_add_ivarlist( classes->B_cls, NULL);
   assert( ! rval);
   rval = mulle_objc_class_add_ivarlist( classes->C_cls, NULL);
   assert( ! rval);

   rval = mulle_objc_class_add_propertylist( classes->A_cls, NULL);
   assert( ! rval);
   rval = mulle_objc_class_add_propertylist( classes->B_cls, NULL);
   assert( ! rval);
   rval = mulle_objc_class_add_propertylist( classes->C_cls, NULL);
   assert( ! rval);
}


static void   test_normal_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_class_search_method( classes->A_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);

   // root meta inherits from root...
   method = mulle_objc_class_search_method( classes->A_meta_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);

   method = mulle_objc_class_search_method( classes->B_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);

   method = mulle_objc_class_search_method( classes->B_meta_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);

   method = mulle_objc_class_search_method( classes->C_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);

   method = mulle_objc_class_search_method( classes->C_meta_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);


   method = mulle_objc_class_search_method( classes->A_cls, bar_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->A_meta_cls, bar_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->B_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) B_bar);

   method = mulle_objc_class_search_method( classes->B_meta_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) B_meta_bar);

   method = mulle_objc_class_search_method( classes->C_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) C_bar);

   method = mulle_objc_class_search_method( classes->C_meta_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) B_meta_bar);
}



static void   test_inhibited_inheritance( struct abc_classes  *classes)
{
   struct _mulle_objc_method    *method;

   method = mulle_objc_class_search_method( classes->A_cls, foo_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) A_foo);

   // root meta inherits from root...
   method = mulle_objc_class_search_method( classes->A_meta_cls, foo_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->B_cls, foo_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->B_meta_cls, foo_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->C_cls, foo_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->C_meta_cls, foo_methodid);
   assert( ! method);


   method = mulle_objc_class_search_method( classes->A_cls, bar_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->A_meta_cls, bar_methodid);
   assert( ! method);

   method = mulle_objc_class_search_method( classes->B_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) B_bar);

   method = mulle_objc_class_search_method( classes->B_meta_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) B_meta_bar);

   method = mulle_objc_class_search_method( classes->C_cls, bar_methodid);
   assert( method);
   assert( method->implementation == (mulle_objc_methodimplementation_t) C_bar);

   method = mulle_objc_class_search_method( classes->C_meta_cls, bar_methodid);
   assert( ! method);
}


static void   _test_simple_inheritance( struct abc_classes  *classes)
{
   create_ABC_classes( classes);
   add_simple_methods( classes);
   add_ABC_classes( classes);

   test_normal_inheritance( classes);

// now change inheritance around and see what happens
   _mulle_objc_class_set_inheritance( classes->A_cls, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_class_set_inheritance( classes->B_cls, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_class_set_inheritance( classes->C_cls, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_class_set_inheritance( classes->A_meta_cls, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_class_set_inheritance( classes->B_meta_cls, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);
   _mulle_objc_class_set_inheritance( classes->C_meta_cls, MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS);

   test_inhibited_inheritance( classes);
}


void   test_simple_inheritance( void)
{
   struct abc_classes   classes;

   _test_simple_inheritance( &classes);
}


