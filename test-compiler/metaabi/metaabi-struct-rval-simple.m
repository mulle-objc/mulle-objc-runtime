//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>

#ifdef JUST_COMPILE
id     mulle_objc_infraclass_alloc_instance( Class);
void   mulle_objc_instance_free( id);
void   *mulle_objc_object_call( id, SEL, void *);

struct mulle_clang_objccompilerinfo
{
   unsigned int   load_version;
   unsigned int   runtime_version;
} __mulle_objc_objccompilerinfo =
{
   17,
   1848
};

#endif



struct duo
{
   float   a;
   float   b;
};


@interface Foo

@property( assign) struct duo  ab;

@end


@implementation Foo

+ (Class) class
{
   return( self);
}

@end


int   main( int argc, const char * argv[])
{
   struct duo   value;
   Foo          *foo;

   foo   = mulle_objc_infraclass_alloc_instance( [Foo class]);
   value = [foo ab];

   // printf( "%g %g", value.a, value.b);

   mulle_objc_instance_free( foo);
   return( 0);
}

