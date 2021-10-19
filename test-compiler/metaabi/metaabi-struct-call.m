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

+ (void) callLargeStruct:(struct large_struct) v
{
   printf( "-callLargeStruct:{ INTPTR_MIN, INTPTR_MAX } %s\n",
      (v.value1 == INTPTR_MIN && v.value2 == INTPTR_MAX) ? "PASS" : "FAIL");
}

@end


int main(int argc, const char * argv[])
{
   struct large_struct   ls;

   ls.value1 = INTPTR_MIN;
   ls.value2 = INTPTR_MAX;
   [Foo callLargeStruct:ls];
   return( 0);
}



