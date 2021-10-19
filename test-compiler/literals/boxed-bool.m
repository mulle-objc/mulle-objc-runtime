#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>

typedef int   BOOL;

enum
{
   YES = 1,
   NO  = 0
};

// @YES @NO is not supported

@interface NSNumber

+ (id) numberWithBOOL:(BOOL) x;
+ (id) numberWithChar:(char) x;
+ (id) numberWithInt:(char) x;

@end


@implementation NSNumber

+ (id) numberWithBOOL:(BOOL) x
{
   printf( "%s\n", x ? "YES" : "NO");
   return( (void *) 1);
}

+ (id) numberWithChar:(char) x
{
   printf( "%s\n", x ? "YES" : "NO");
   return( (void *) 1);
}

+ (id) numberWithInt:(char) x
{
   printf( "%s\n", x ? "YES" : "NO");
   return( (void *) 1);
}

@end


main()
{
   id   foo;

   foo = @(YES);
   if( ! foo)
      return( 1);

   foo = @(NO);
   if( ! foo)
      return( 1);
   // just be unhappy if it compiles
   return( 0);
}

