//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <limits.h>
#include <stdio.h>


typedef struct _mulle_objc_infraclass *Class;

@interface Foo

+ (Class) class;

@end

@implementation Foo

+ (Class) class
{
   return( self);
}

- (instancetype) retain
{
   return( _mulle_objc_object_retain( self, _cmd, self));
}


- (void) release
{
   _mulle_objc_object_release( self, _cmd, self);
}

@end


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_object   *a;
   struct _mulle_objc_object   *b;

   assert( MULLE_OBJC_SLOW_RELEASE != MULLE_OBJC_NEVER_RELEASE);
   assert( MULLE_OBJC_INLINE_RELEASE != MULLE_OBJC_NEVER_RELEASE);

   a = mulle_objc_infraclass_alloc_instance( [Foo class]);
   assert( mulle_objc_object_get_retaincount( a) == 1);

   b = mulle_objc_object_call_retain( a);
   assert( a == b);
   assert( mulle_objc_object_get_retaincount( a) == 2);

   mulle_objc_object_call_release( a);
   assert( mulle_objc_object_get_retaincount( a) == 1);

   mulle_objc_instance_free( a);

   return( 0);
}

