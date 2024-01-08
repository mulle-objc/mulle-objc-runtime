//
//  main.m
//  test-rval
//
//  Created by Nat! on 09/11/15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>


struct rect
{
   double   x, y, w, h;
};



@interface Foo
@end


//
// this test is easier to understand in .ir and .s if something goes wrong
// inspect with: mulle-sde test run --assembler --ir region-rval.m
//
@implementation Foo

+ (struct rect) rect:(double) x
{
   return( (struct rect) { 1.0 * x , 2.0 * x, 3.0 * x, 4.0 * x });
}

@end


int  main( void)
{
   struct rect   r1;
   struct rect   r2;

   r1 = [Foo rect:1.0];
   r2 = [Foo rect:2.0];

   // return 0 if matches
   return( (r1.x + r1.y + r1.w + r1.h == 1.0 * (1.0 + 2.0 + 3.0 + 4.0)) &&
           (r2.x + r2.y + r2.w + r2.h == 2.0 * (1.0 + 2.0 + 3.0 + 4.0))
            ? 0
            : 1);
}
