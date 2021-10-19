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

struct larger_struct
{
   intptr_t   value1;
   intptr_t   value2;
   intptr_t   value3;
   intptr_t   value4;
   intptr_t   value5;
   intptr_t   value6;
};


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( self);
}


- (struct larger_struct) callLargerStruct:(struct larger_struct) p
{
   struct larger_struct  x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   x.value1 = p.value6;
   x.value2 = p.value5;
   x.value3 = p.value4;
   x.value4 = p.value3;
   x.value5 = p.value2;
   x.value6 = p.value1;

   return( x);
}


- (struct large_struct) callLargeStruct:(struct large_struct) p
{
   struct large_struct  x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   x.value1 = p.value2;
   x.value2 = p.value1;

   return( x);
}


- (struct large_struct) callLargeStruct
{
   struct large_struct  x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   x.value1 = INTPTR_MIN;
   x.value2 = INTPTR_MAX;

   return( x);
}

@end


int main( int argc, const char * argv[])
{
   struct large_struct   ls;
   struct larger_struct  lls;
   Foo                   *foo;
   Foo                   *nothing;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);



   ls.value1 = INTPTR_MAX;
   ls.value2 = INTPTR_MIN;

   ls = [foo callLargeStruct];
   if( ls.value1 != INTPTR_MIN || ls.value2 != INTPTR_MAX)
   {
      fprintf( stderr, "FAIL1\n");
      return( 1);
   }

   ls.value1 = INTPTR_MAX;
   ls.value2 = INTPTR_MIN;

   // flips it around
   ls = [foo callLargeStruct:ls];
   if( ls.value1 != INTPTR_MIN || ls.value2 != INTPTR_MAX)
   {
      fprintf( stderr, "FAIL2\n");
      return( 1);
   }


   // flips it around

   memset( &lls, 0, sizeof( lls));

   lls.value1 = INTPTR_MAX;
   lls.value6 = INTPTR_MIN;

   lls = [foo callLargerStruct:lls];
   if( lls.value1 != INTPTR_MIN || lls.value6 != INTPTR_MAX)
   {
      fprintf( stderr, "FAIL3\n");
      return( 1);
   }

   mulle_objc_instance_free( foo);

   return( 0);
}



