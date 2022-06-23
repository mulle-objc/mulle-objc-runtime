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

   // theory: first call needs to warm up the cache so there - could -
   //         be a difference in stack pointers between the first and
   //         second call
   [Foo callWithA:1.0
                b:2.0];

   sp = 0;

   // now the stackpointer should remain stable
   for( i = 0; i < 999; i++)
   {
      osp = sp;
      sp = [Foo callWithA:1.0
                        b:2.0].pointer;
      if( i != 0 && sp != osp)
      {
         fprintf( stderr, "FAIL\n");
         return( 1);
      }
   }
   return( 0);
}

