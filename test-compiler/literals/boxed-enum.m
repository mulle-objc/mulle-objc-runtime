#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface NSNumber

+ (id) numberWithInt:(int) x;

@end


@implementation NSNumber

+ (id) numberWithInt:(int) x
{
   printf( "%d\n", x);
   return( (id) 1);
}

@end


enum {
   Red = 0,
   Green = 1,
   Blue = 2
};

int  main( void)
{
   id   foo;

   foo = @( Blue);
   // just be happy that it compiles :)
   return( foo ? 0 : 1);
}

