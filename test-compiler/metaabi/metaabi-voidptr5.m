//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Foo
@end


@implementation Foo


// metaABI wise it's known that (a) we get a _param block for a double
// (b) that block is at least 5 void * wide
// (c) we can reuse that block if we don't care about its contents anymore
// (d) a return value will be written into _param on return, but that's of
//     no concern here.

+ (void) callDouble:(double) value
          keepAlive:(double *) x
          keepAlive:(double *) y
{
   void  **p = (void *) _param;

   p[ 0] = (void *) 1;
   p[ 1] = (void *) 2;
   p[ 2] = (void *) 3;
   p[ 3] = (void *) 4;
   p[ 4] = (void *) 5;
}

@end


int   main( int argc, const char * argv[])
{
   double  x;
   double  y;

   x  = -1.0;
   [Foo callDouble:-2.0
         keepAlive:&x
         keepAlive:&y];
   if( x != -1.0)
   {
      fprintf( stderr, "x (-1.0) got clobbered\n");
      return( 1);
   }

   // try to trick the compiler in putting y "behind" param, might not work though
   // (does not on x86_64 but it doesn't matter, as 'x' is behind the param
   // block waiting to be clobbered (with --debug at least))
   x = -3.0;
   y = -4.0;
   [Foo callDouble:-5.0
         keepAlive:&x
         keepAlive:&y];
   if( x != -3.0)
   {
      fprintf( stderr, "x (-3.0) got clobbered\n");
      return( 1);
   }
   if( y != -4.0)
   {
      fprintf( stderr, "y (-4.0) got clobbered\n");
      return( 1);
   }
   return( 0);
}

