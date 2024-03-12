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

+ (void *) callWithA:(double) a
                   b:(double) b
                   c:(double) c
                   d:(double) d
                   e:(double) e
                   f:(double) f
{
   void   *p = 0;
   return( &p);
}

@end


int   main( int argc, const char * argv[])
{
   void   *osp;
   void   *sp;
   int    i;

   // theory: first call needs to run initialize to setup cache which will be
   //         empty.
   //         second call will then fill the cache
   //         third call will be from cached and then the sp should be stable
   //         starting from the fourth call
   sp = 0;

   // now the stackpointer should remain stable
   for( i = 1; i <= 1000; i++)
   {
      osp = sp;
      sp  = [Foo callWithA:1.0
                         b:2.0
                         c:3.0
                         d:4.0
                         e:5.0
                         f:6.0];
      if( i >= 4 && sp != osp)
      {
         fprintf( stderr, "FAIL\n");
         return( 1);
      }
   }
   return( 0);
}

