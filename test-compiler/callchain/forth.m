#ifdef __MULLE_OBJC__
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#endif

#include <stdio.h>

@implementation Foo

+ (Class) class
{
   return( self);
}


+ (void) chain:(id) arg
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation Foo( X)

+ (void) chain:(id) arg
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation Foo( Y)


+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency   dependencies[] =
   {
      { @selector( Foo), @selector( X) },
      { MULLE_OBJC_NO_CLASSID, MULLE_OBJC_NO_CATEGORYID }
   };

   return( dependencies);
}


+ (void) chain:(id) arg
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


int   main( void)
{
   Class  cls;

   cls = [Foo class];
   mulle_objc_object_call_chained_forth( cls, @selector( chain:), cls);
   return( 0);
}
