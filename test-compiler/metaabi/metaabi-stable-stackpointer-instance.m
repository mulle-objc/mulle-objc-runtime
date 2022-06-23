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

+ (id) new
{
   return( _mulle_objc_infraclass_alloc_instance( self));
}


- (void) dealloc
{
   _mulle_objc_instance_free( self);
}


- (void *) callWithA:(double) a
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
   Foo    *foo;

   // theory: first call needs to warm up the cache so there - could -
   //         be a difference in stack pointers between the first and
   //         second call
   foo = [Foo new];
   {
      [foo callWithA:1.0
                   b:2.0
                   c:3.0
                   d:4.0
                   e:5.0
                   f:6.0];

      sp = 0;

      // now the stackpointer should remain stable
      for( i = 0; i < 999; i++)
      {
         osp = sp;
         sp  = [foo callWithA:1.0
                            b:2.0
                            c:3.0
                            d:4.0
                            e:5.0
                            f:6.0];
         if( i != 0 && sp != osp)
         {
            fprintf( stderr, "FAIL\n");
            return( 1);
         }
      }
   }
   [foo dealloc];

   return( 0);
}

