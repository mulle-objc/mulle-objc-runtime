#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface NSNumber

+ (id) numberWithInt:(int) x;

@end


@implementation NSNumber

+ (id) numberWithInt:(int) x
{
   printf( "%d\n", x);
}

@end


main()
{
   id   foo;

   foo = @1;
   // just be happy that it compiles :)
}

