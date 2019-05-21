#ifdef __MULLE_OBJC__
# include <mulle-objc-runtime/mulle-objc-runtime.h>
static char  *get_class_name( Class self)
{
   return( _mulle_objc_infraclass_get_name( self));
}
#else
# import <Foundation/Foundation.h>
static char  *get_class_name( Class self)
{
   return( [NSStringFromClass( self) UTF8String]);
}
#endif




@interface A
@end


@implementation A

+ (void) initialize
{
   printf( "%s :: %s\n", get_class_name( self), __PRETTY_FUNCTION__);
}


+ (void) nop
{
}

@end



@interface B : A
@end

@implementation B
@end


@interface C : A
@end

@implementation C
@end


@interface D : C
@end

@implementation D
@end



int  main( void)
{
   [B nop];
   [D nop];
   return( 0);
}
