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
@end


@implementation Foo

+ (void) mulleInitWithRelativeTimeInterval:(double) seconds
                            repeatInterval:(double) repeatSeconds
                                    target:(id) target
                                  selector:(SEL) selector
                                  userInfo:(id) userInfo
                fireUsesUserInfoAsArgument:(BOOL) flag
{
   printf( "%g %g %p %p %p %p\n",
               seconds,
               repeatSeconds,
               (void *) target,
               (void *) (uintptr_t) selector,
               userInfo,
               (void *) flag);
}

@end


int main( int argc, const char * argv[])
{
   [Foo mulleInitWithRelativeTimeInterval:1.0
                           repeatInterval:2.0
                                   target:(id) 3
                                 selector:(SEL) 4
                                 userInfo:(void *) 5
               fireUsesUserInfoAsArgument:6];
   return( 0);
}



