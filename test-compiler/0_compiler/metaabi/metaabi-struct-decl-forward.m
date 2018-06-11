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

+ (struct large_struct) returnLargeStruct;

@end


struct large_struct
{
   intptr_t   value1;
   intptr_t   value2;
};


@implementation Foo

+ (struct large_struct) returnLargeStruct
{
   struct large_struct   ls;

   ls.value1 = INTPTR_MIN;
   ls.value2 = INTPTR_MAX;

   return( ls);
}

@end


int main(int argc, const char * argv[])
{
   struct large_struct   ls;

   ls = [Foo returnLargeStruct];

   return( ! (ls.value1 == INTPTR_MIN && ls.value2 == INTPTR_MAX));
}



