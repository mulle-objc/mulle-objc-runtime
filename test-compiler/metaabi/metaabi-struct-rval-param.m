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

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}


static struct large_struct   do_something( struct large_struct p)
{
   intptr_t   x;

   x        = p.value1;
   p.value1 = p.value2;
   p.value2 = x;
   return( p);
}


- (struct large_struct) getLargeStruct
{
   struct large_struct  p;

   p.value1 = 1;
   p.value2 = 2;

   return( p);
}

- (void *) getSelf:(void *) y :(void *) z
{
   return( self);
}

- (int) getInt:(void *) y
{
   return( 0.0);
}



- (float) getFloat
{
   return( 0.0);
}


- (struct large_struct) callLargeStruct:(struct large_struct) p
{
   struct large_struct  x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   x = do_something( p);
   return( p);
}

@end


int main( int argc, const char * argv[])
{
   struct large_struct   ls;
   struct large_struct   ls2;
   Foo                   *foo;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   ls.value1 = INTPTR_MAX;
   ls.value2 = INTPTR_MIN;

   ls2 = [foo callLargeStruct:ls];
   if( ls.value2 != INTPTR_MIN || ls.value1 != INTPTR_MAX)
   {
      fprintf( stderr, "FAIL1\n");
      return( 1);
   }

   mulle_objc_instance_free( foo);

   return( 0);
}



