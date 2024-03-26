#ifdef __MULLE_OBJC__
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#endif

#include <stdio.h>


@implementation Foo

+ (void) printClassName
{
   printf( "%s\n", __OBJC_CLASS__);
}


+ (void) printClassID
{
   printf( "%x\n", (unsigned int) __MULLE_OBJC_CLASSID__);
}

@end


@implementation Foo ( Bar)

+ (void) printCategoryName
{
   printf( "%s\n", __OBJC_CATEGORY__);
}

+ (void) printCategoryID
{
   printf( "%x\n", (unsigned int) __MULLE_OBJC_CATEGORYID__);
}

@end


int   main( void)
{
   [Foo printClassName];
   [Foo printClassID];
   [Foo printCategoryName];
   [Foo printCategoryID];

   return( 0);
}
