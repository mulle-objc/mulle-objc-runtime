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

+ (void) a:(int) a
         b:(int) b
{
   printf( "%d %d\n", _param->a, _param->b);
}

@end


int main( int argc, const char * argv[])
{
   [Foo a:(SEL) 18 b:(SEL) 48];
   return( 0);
}



