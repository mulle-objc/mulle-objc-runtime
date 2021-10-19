#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface A
@end

@interface B : A
@end

@interface C : B
@end


@implementation A

+ (void) a0
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


+ (void) a1
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


+ (void) a2
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end



@implementation B

+ (void) a1
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   [super a1];
}


+ (void) a2
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   [super a2];
}


+ (void) b0
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


+ (void) b1
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation C

+ (void) a0
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   [super a0];
}

+ (void) a1
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   [super a1];
}


+ (void) b0
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   [super b0];
}

@end



int  main( void)
{
   if( mulle_objc_global_check_universe( __MULLE_OBJC_UNIVERSENAME__) != mulle_objc_universe_is_ok)
      return( 1);

   printf( "A:\n");
   [A a0];
   [A a1];
   [A a2];

   printf( "\nB:\n");
   [B a0];
   [B a1];
   [B a2];
   [B b0];
   [B b1];

   printf( "\nC:\n");
   [C a0];
   [C a1];
   [C a2];
   [C b0];
   [C b1];

   return(0);
}
