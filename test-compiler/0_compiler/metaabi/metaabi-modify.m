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


#pragma clang diagnostic ignored "-Wobjc-root-class"

typedef double   NSTimeInterval;

@interface NSTimeZone

- (NSTimeInterval)  secondsFromGMT;

@end


@interface NSDate
{
   NSTimeInterval   _interval;
}
@end


@implementation NSDate

+ (instancetype) alloc
{
   return( _mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}

- (void) dealloc
{
   return( _mulle_objc_instance_free( self));
}


- (instancetype) initWithTimeIntervalSince1970:(NSTimeInterval) interval
{
   _interval = interval;
   return( self);
}

@end


@implementation NSDate (NSDateFormatter)


- (instancetype) initWithTimeintervalSince1970:(NSTimeInterval) interval
                                      timeZone:(NSTimeZone *) timeZone
{
   interval -= [timeZone secondsFromGMT];
   return( [self initWithTimeIntervalSince1970:interval]);
}

@end


int   main( void)
{
   [[[NSDate alloc] initWithTimeintervalSince1970:1848.0
                                         timeZone:NULL] dealloc];
   return( 0);
}

