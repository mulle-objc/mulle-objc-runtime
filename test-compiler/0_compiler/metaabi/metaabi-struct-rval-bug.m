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


struct large_struct
{
   intptr_t   value1;
   intptr_t   value2;
};


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( self);
}

- (struct large_struct) callLargeStruct
{
   struct large_struct  x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   x.value1 = INTPTR_MIN;
   x.value2 = INTPTR_MAX;

   return( x);
}

- (int) testCall
{
   struct large_struct  ls;
   ls.value1 = INTPTR_MAX;
   ls.value2 = INTPTR_MIN;

   return( [self callLargeStruct].value1 != INTPTR_MIN);
}

@end


int main( int argc, const char * argv[])
{
   Foo  *foo;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   if( [foo testCall])
   {
      fprintf( stderr, "FAIL\n");
      return( 1);
   }

   mulle_objc_instance_free( foo);

   return( 0);
}



