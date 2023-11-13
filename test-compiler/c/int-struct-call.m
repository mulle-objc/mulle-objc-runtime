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


struct small_struct
{
   intptr_t   value1;
   intptr_t   value2;
};

struct larger_struct
{
   intptr_t   value1;
   intptr_t   value2;
   intptr_t   value3;
   intptr_t   value4;
};


// just check that C calls work as expected

struct small_struct  foo( struct small_struct a, struct larger_struct b, intptr_t c)
{
   printf( "foo( { %td %td }, { %td %td %td %td }, %td) -> ",
                  a.value1,
                  a.value2,
                  b.value1,
                  b.value2,
                  b.value3,
                  b.value4,
                  c);
   return( ((struct small_struct) { .value1 = 100, .value2 = 200 }));
}


int main( int argc, const char * argv[])
{
   struct small_struct   r;

   r = foo( ((struct small_struct) { .value1 = 1, .value2 = 2 }),
            ((struct larger_struct) { .value1 = 3, .value2 = 4, .value3 = 5, .value4 = 6 }),
            7);
   printf( "{ %td %td }\n",
                  r.value1,
                  r.value2);
   return( 0);
}



