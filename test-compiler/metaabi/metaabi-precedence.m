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
{
   SEL  selector;
   SEL  _selector;
}
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( self);
}


- (void) a:(SEL) _selector
         b:(SEL) selector
{
   printf( "%td %td\n", (intptr_t) _selector, (intptr_t) selector);
}

@end


int main( int argc, const char * argv[])
{
   Foo    *foo;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   [foo a:(SEL) 18 b:(SEL) 48];
   mulle_objc_instance_free( foo);

   return( 0);
}



