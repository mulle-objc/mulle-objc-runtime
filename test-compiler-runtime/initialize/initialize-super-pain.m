#ifdef __MULLE_OBJC__
# include <mulle-objc-runtime/mulle-objc-runtime.h>
static char  *get_class_name( Class self)
{
   return( _mulle_objc_infraclass_get_name( (struct _mulle_objc_infraclass *) self));
}
#else
# import <Foundation/Foundation.h>
static char  *get_class_name( Class self)
{
   return( [NSStringFromClass( self) UTF8String]);
}
#endif

#pragma clang diagnostic ignored "-Wobjc-root-class"

@interface A
@end


@interface B : A
@end

@interface C : B
@end



@implementation A

+ (Class) class
{
   return( self);
}

+ (void) initialize
{
   printf( "%s :: %s\n", get_class_name( self), __PRETTY_FUNCTION__);
}

@end


@implementation B

+ (void) initialize
{
   [super initialize];
   printf( "%s :: %s\n", get_class_name( self), __PRETTY_FUNCTION__);
}

@end


@implementation C

+ (void) initialize
{
   [super initialize];
   printf( "%s :: %s\n", get_class_name( self), __PRETTY_FUNCTION__);
}

@end



int  main( void)
{
   [B class];
   [C class];
   return( 0);
}
