//
//  test_simple_inheritance.h
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc/mulle_objc.h>


struct abc_classes
{
   struct _mulle_objc_infraclass   *A_infra;
   struct _mulle_objc_infraclass   *B_infra;
   struct _mulle_objc_infraclass   *C_infra;

   struct _mulle_objc_metaclass   *A_meta;
   struct _mulle_objc_metaclass   *B_meta;
   struct _mulle_objc_metaclass   *C_meta;
};


void   create_ABC_classes( struct abc_classes *classes);
void   add_ABC_classes( struct abc_classes *classes);
void   add_simple_methods( struct abc_classes  *classes);
void   test_simple_inheritance( void);


void   *A_foo( void *self, mulle_objc_methodid_t _cmd, void *_params);
void   *B_bar( void *self, mulle_objc_methodid_t _cmd, void *_params);
void   *B_meta_bar( void *self, mulle_objc_methodid_t _cmd, void *_params);
void   *C_bar( void *self, mulle_objc_methodid_t _cmd, void *_params);

extern struct _mulle_objc_method   *A_foo_method;
extern struct _mulle_objc_method   *B_bar_method;
extern struct _mulle_objc_method   *B_meta_bar_method;
extern struct _mulle_objc_method   *C_bar_method;

extern struct _mulle_objc_methodlist  A_i_list;
extern struct _mulle_objc_methodlist  B_c_list;
extern struct _mulle_objc_methodlist  B_i_list;
extern struct _mulle_objc_methodlist  C_i_list;
