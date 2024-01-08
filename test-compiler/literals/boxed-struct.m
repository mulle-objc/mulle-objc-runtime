#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


// https://clang.llvm.org/docs/ObjectiveCLiterals.html

@interface NSValue

+ (id) valueWithBytes:(void *) bytes
             objCType:(char *) type;

@end


@implementation NSValue

+ (id) valueWithBytes:(void *) bytes
             objCType:(char *) type
{
   printf( "%s\n", type);
   return( (id) 1);
}

@end


struct __attribute__((objc_boxable)) ab_t
{
   int   a;
   int   b;
};


int  main( void)
{
   struct ab_t   ab = { 18, 48 };
   id            foo;

   foo = @( ab);
   // just be happy that it compiles :)
   return( foo ? 0 : 1);
}

