#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface A
@end


@interface A( B)
@end


@implementation A

+ (void) bar
{
   printf( "%s: %s( %s)\n", __PRETTY_FUNCTION__, __OBJC_CLASS__, __OBJC_CATEGORY__);
}

@end


void  alone()
{
   printf( "%s: %s( %s)\n", __PRETTY_FUNCTION__, __OBJC_CLASS__, __OBJC_CATEGORY__);
}


@implementation A( B)

+ (void) baz
{
   printf( "%s: %s( %s)\n", __PRETTY_FUNCTION__, __OBJC_CLASS__, __OBJC_CATEGORY__);

//
// this doesn't work though as __OBJC_CLASS__ is a keyword and
// would returns a string like "A":
//
//   printf( "%08x\n", @selector( __OBJC_CLASS__));
}

@end


main()
{
   alone();
   [A bar];
   [A baz];

   return( 0);
}

