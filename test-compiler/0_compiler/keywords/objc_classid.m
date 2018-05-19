#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface A
@end


@interface A( B)
@end


@implementation A

// the way clang works is this:  it collects all tokens
// from @implementation to @end. This already scrutinizes
// things like self->value, which are OK.
// Now when @end is found, it actually parses the tokens
// A function foo is not tied to 'A' anymore, so the
// relationship is lost. But __MULLE_OBJC_CLASSID__ is evaluated
// now (too late). Can't find A and therefore returns 0
//
#if DOES_NOT_WORK_IN_CLANG
void  foo( void)
{
   printf( "%s: %08x,%08x -- %08x,%08x\n",
         __PRETTY_FUNCTION__,
         __MULLE_OBJC_CLASSID__, __MULLE_OBJC_CATEGORYID__,
         @selector( A), MULLE_OBJC_NO_CATEGORYID);
}
#endif


+ (void) bar
{
   printf( "%s: %08x,%08x -- %08x,%08x\n",
         __PRETTY_FUNCTION__,
         __MULLE_OBJC_CLASSID__, __MULLE_OBJC_CATEGORYID__,
         @selector( A), MULLE_OBJC_NO_CATEGORYID);
}

@end


void  alone()
{
   printf( "%s: %08x,%08x -- %08x,%08x\n",
         __PRETTY_FUNCTION__,
         __MULLE_OBJC_CLASSID__, __MULLE_OBJC_CATEGORYID__,
         MULLE_OBJC_NO_CLASSID, MULLE_OBJC_NO_CATEGORYID);
}


@implementation A( B)

+ (void) baz
{
   printf( "%s: %08x,%08x -- %08x,%08x\n",
         __PRETTY_FUNCTION__,
         __MULLE_OBJC_CLASSID__, __MULLE_OBJC_CATEGORYID__,
         @selector( A), @selector( B));
}

@end


main()
{
   alone();
   [A bar];
   [A baz];
   return( 0);
}

