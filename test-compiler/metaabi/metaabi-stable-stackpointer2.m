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

struct rval
{
   double  value;
   void    *pointer;
};


+ (struct rval) callWithA:(double) a
                        b:(double) b
{
   struct rval   rval;

   rval.value   = 12.0;
   rval.pointer = &rval;

   return( rval);
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
      sp = [Foo callWithA:1.0
                        b:2.0].pointer;
      if( i >= 4 && sp != osp)
      {
         fprintf( stderr, "FAIL\n");
         return( 1);
      }
   }
   return( 0);
}

