#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface NSDictionary

+ (id) dictionaryWithObjects:(id *) objects
                     forKeys:(id *) keys
                       count:(long) count;
@end


@implementation NSDictionary

+ (id) dictionaryWithObjects:(id *) objects
                     forKeys:(id *) keys
                       count:(long) count
{
   printf( "%ld\n", count);
   return( nil);
}

@end


int   main( void)
{
   id   foo;

   foo = @{ @"foo" : @"bar" };
   // just be happy that it compiles :)
}

