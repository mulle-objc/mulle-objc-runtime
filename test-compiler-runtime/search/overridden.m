#import <mulle-objc-runtime/mulle-objc-runtime.h>


@interface A
@end

@class P;
@protocol P
@end
@interface P < P>
@end

@class Q;
@protocol Q
@end
@interface Q < Q>
@end

@interface B : A < P, Q>
@end
@interface B( C)
@end


@implementation A
+ (id) new
{
   return( (id) _mulle_objc_infraclass_alloc_instance( self));
}
+ (Class) class
{
   return( self);
}
- (void) dealloc
{
   _mulle_objc_instance_free( self);
}
+ (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@implementation P
+ (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@implementation Q
+ (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@implementation B
+ (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end
@implementation B( C)
+ (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) foo
{
   printf( "%s\n", __PRETTY_FUNCTION__);
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

   // not sure why valgrind complains otherwise ?
   // _mulle_objc_searcharguments_init_overridden inits all values...
   memset( &args, 0xFF, sizeof( args));
   memset( &before, 0xFF, sizeof( before));

   _mulle_objc_searcharguments_init_overridden( &args, methodsel, classsel, categorysel);

   before = args;
   method = mulle_objc_class_search_method( &infraclass->base, &args, infraclass->base.inheritance, NULL);
   if( ! method)
      abort();

   imp = _mulle_objc_method_get_implementation( method);
   mulle_objc_implementation_invoke( imp, obj, methodsel, obj);

   assert( ! memcmp( &args, &before, sizeof( args)));

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
   infraclass = [B class];
   metaclass  = _mulle_objc_infraclass_get_metaclass( infraclass);

   test_overridden( b, @selector( foo), @selector( B), @selector( C), infraclass, metaclass);
   test_overridden( b, @selector( foo), @selector( B), 0, infraclass, metaclass);
   test_overridden( b, @selector( foo), @selector( Q), 0, infraclass, metaclass);
   test_overridden( b, @selector( foo), @selector( P), 0, infraclass, metaclass);

   [b dealloc];
}