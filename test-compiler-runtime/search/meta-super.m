#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


@interface A
@end

@implementation A
+ (void) x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



@interface B : A
@end


@implementation B
@end



@class X;
@protocol X
@end
@interface X <X>
@end


@implementation X
+ (void) x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) y
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



@interface C < X>
@end

@implementation C
+ (void) y
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



@interface D : C
@end


@implementation D
@end


int  test( SEL classid, SEL methodid)
{
   struct _mulle_objc_infraclass        *infra;
   struct _mulle_objc_metaclass         *meta;
   struct _mulle_objc_method            *method;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   struct _mulle_objc_universe          *universe;

   universe = mulle_objc_global_get_universe( MULLE_OBJC_DEFAULTUNIVERSEID);
   infra    = mulle_objc_universe_lookup_infraclass_nofail( universe, classid);
   meta     = _mulle_objc_infraclass_get_metaclass( infra);
   _mulle_objc_searcharguments_init_default( &search, methodid);

   method   = mulle_objc_class_search_method( &meta->base,
                                              &search,
                                              meta->base.inheritance,
                                              &result);
   if( ! method)
      return( 1);

   mulle_objc_implementation_invoke( _mulle_objc_method_get_implementation( method),
                                     infra,
                                     _mulle_objc_method_get_methodid( method),
                                     infra);
   return( 0);
}


// stupid little test, because i messed something up
int   main()
{
   if( test( @selector( B), @selector( x)))
      return( 1);
   if( test( @selector( D), @selector( x)))
      return( 1);
   if( test( @selector( D), @selector( y)))
      return( 1);

   return( 0);
}
