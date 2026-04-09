#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface NSArray

+ (id) arrayWithObjects:(id *) objects
                  count:(long) count;
@end


@implementation NSArray

+ (id) arrayWithObjects:(id *) objects
                  count:(long) count
{
   printf( "%ld\n", count);
   return( (id) 0);
}

@end


int   main( void)
{
   id   foo;

//   [NSArray arrayWithObjects:&foo
//                       count:1];
   foo = @[ @"foo", @"bar" ];
   // just be happy that it compiles :)
}

