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


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( self);
}


- (double) callDouble
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( 18.48);
}

@end


int main( int argc, const char * argv[])
{
   Foo    *foo;
   Foo     *nothing;
   double  x;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   // keep nil code alive (voodoo)
   nothing = foo;
   if( argc != 1848)
      nothing = 0;

   x  = 18.48;
   x += [nothing callDouble];
   if( x != 18.48)
   {
      fprintf( stderr, "FAIL\n");
      return( 1);
   }

   mulle_objc_instance_free( foo);

   return( 0);
}



