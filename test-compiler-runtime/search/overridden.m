#import <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface A
@end

@protocol_class P;
@protocol_interface P
@end

@protocol_class Q;
@protocol_interface Q
@end

@interface B : A < P, Q>
@end
@interface B( C)
@end


@implementation A
+ (id) new
{
   return( (id) _mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}
+ (Class) class
{
   return( (Class) self);
}
- (void) dealloc
{
   _mulle_objc_instance_free( self);
}
+ (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@protocol_implementation P
+ (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@protocol_implementation Q
+ (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@implementation B
+ (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end
@implementation B( C)
+ (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


static void   test_overridden( id obj,
                               SEL methodsel,
                               SEL classsel,
                               SEL categorysel,
                               struct _mulle_objc_infraclass *infraclass,
                               struct _mulle_objc_metaclass *metaclass)

{
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_searcharguments    before;
   struct _mulle_objc_method             *method;
   mulle_objc_implementation_t           imp;

   args   = mulle_objc_searcharguments_make_overridden( methodsel, classsel, categorysel);
   before = args;
   method = mulle_objc_class_search_method( &infraclass->base, &args, infraclass->base.inheritance, NULL);
   if( ! method)
      abort();

   imp = _mulle_objc_method_get_implementation( method);
   mulle_objc_implementation_invoke( imp, obj, methodsel, obj);

#ifndef MULLE_TEST_VALGRIND
   assert( ! memcmp( &args, &before, sizeof( args)));
#endif

   method = mulle_objc_class_search_method( &metaclass->base, &args, metaclass->base.inheritance, NULL);
   if( ! method)
      abort();

   imp = _mulle_objc_method_get_implementation( method);
   mulle_objc_implementation_invoke( imp, obj, methodsel, obj);
}


int   main()
{
   B                              *b;
   struct _mulle_objc_infraclass  *infraclass;
   struct _mulle_objc_metaclass   *metaclass;
   struct _mulle_objc_universe    *universe;

#ifdef __MULLE_OBJC_UNIVERSENAME__
   universe = mulle_objc_global_get_universe( mulle_objc_universeid_from_string( __MULLE_OBJC_UNIVERSENAME__));
#else
   universe = mulle_objc_global_get_universe( MULLE_OBJC_DEFAULTUNIVERSEID);
#endif

   b          = [B new];
   infraclass = (struct _mulle_objc_infraclass *) [B class];
   metaclass  = _mulle_objc_infraclass_get_metaclass( infraclass);

   test_overridden( b, @selector( foo), @selector( B), @selector( C), infraclass, metaclass);
   test_overridden( b, @selector( foo), @selector( B), 0, infraclass, metaclass);
   test_overridden( b, @selector( foo), @selector( Q), 0, infraclass, metaclass);
   test_overridden( b, @selector( foo), @selector( P), 0, infraclass, metaclass);

   [b dealloc];
}
