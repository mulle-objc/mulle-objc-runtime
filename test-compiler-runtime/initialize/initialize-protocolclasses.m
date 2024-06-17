# include <mulle-objc-runtime/mulle-objc-runtime.h>



static void   trace( Class self, char *name)
{
   mulle_printf( "%s -> %s\n", _mulle_objc_infraclass_get_name( (struct _mulle_objc_infraclass *) self),
                               name);
}


#define METHOD_NAME  ((char *) __PRETTY_FUNCTION__)

@interface A
@end

@interface B : A
@end


@protocol C
@end
@interface C <C>
@end

@protocol D
@end
@interface D <D>
@end


@interface E < C, D>
@end


@interface F : B < C, D>
@end


@interface G : A
@end


@interface H : A
@end


@protocol I
@end
@interface I <I>
@end

@protocol J
@end
@interface J <J>
@end


@interface K < I, J>
@end


@interface L : K < C, D>
@end


@interface M : L < I>
@end




@implementation A
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end


@implementation B
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}

@end

@implementation C
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}

@end

@implementation D
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end

@implementation E
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end


@implementation F
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end

@implementation G
// has no +initialize
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end


@implementation H
+ (void) initialize
{
   [super initialize];
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end


@implementation I
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end

@implementation J
// has none
//+ (void) initialize
//{
//   trace( self, METHOD_NAME);
//}
+ (void) nop
{
   trace( self, METHOD_NAME);
}

@end

@implementation K
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end

@implementation L
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end

@implementation M
+ (void) initialize
{
   trace( self, METHOD_NAME);
}
+ (void) nop
{
   trace( self, METHOD_NAME);
}
@end




int  main( void)
{
   [A nop];
   mulle_printf( "\n");
   [B nop];
   mulle_printf( "\n");
   [E nop];
   mulle_printf( "\n");
   [F nop];
   mulle_printf( "\n");
   [G nop];
   mulle_printf( "\n");
   [H nop];
   mulle_printf( "\n");

   // don't do 'K'
   [L nop];
   mulle_printf( "\n");
   [M nop];
   mulle_printf( "\n");
   return( 0);
}
