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


- (float) callFloat
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( 18.48f);
}

@end


int main( int argc, const char * argv[])
{
   Foo    *foo;
   float  x;

   foo = 0;

   // keep nil code alive (voodoo)
   x  = 18.48f;
   x += [foo callFloat];
   if( x != 18.48f)
   {
      fprintf( stderr, "FAIL %.2f\n", x);
      return( 1);
   }

   return( 0);
}



