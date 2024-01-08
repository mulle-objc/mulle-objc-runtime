#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface NSArray

+ (id) arrayWithObjects:(id *) objects
                  count:(long) count;
@end


@implementation NSArray

+ (id) arrayWithObjects:(id *) objects
                  count:(long) count
{
   printf( "%ld\n", count);
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

