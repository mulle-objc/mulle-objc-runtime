#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


struct opaque_struct;

@interface Foo
{
   struct opaque_struct  *p;  // will signature to ^{opaque_struct=}
}
@end


@implementation Foo

+ (Class) class
{
   return( self);
}

@end


// no longer opaque
struct opaque_struct
{
   int  foo;
};


@interface Bar
{
   struct opaque_struct  *p;  // will signature to ^{opaque_struct=i}
}
@end


@implementation Bar

+ (Class) class
{
   return( self);
}

@end

static mulle_objc_walkcommand_t
   print_ivar( struct _mulle_objc_ivar *ivar,
               struct _mulle_objc_infraclass *infra,
               void *userinfo)
{
   struct bouncy_info   *info;

   printf( "%s: %s\n", _mulle_objc_ivar_get_name( ivar), _mulle_objc_ivar_get_signature( ivar));
   return( mulle_objc_walk_ok);
}


int   main( int argc, char *argv[])
{
   mulle_objc_hash_t   foo_hash;
   mulle_objc_hash_t   bar_hash;

   foo_hash = _mulle_objc_infraclass_get_ivarhash( (struct _mulle_objc_infraclass *) [Foo class]);
   bar_hash = _mulle_objc_infraclass_get_ivarhash( (struct _mulle_objc_infraclass *) [Bar class]);

   // it's not really desirable that its different now is it ?
   if( foo_hash == bar_hash)
      printf( "SAME\n");
   else
   {
      _mulle_objc_infraclass_walk_ivars( (struct _mulle_objc_infraclass *) [Foo class], 0, print_ivar, NULL);
      _mulle_objc_infraclass_walk_ivars( (struct _mulle_objc_infraclass *) [Bar class], 0, print_ivar, NULL);
      printf( "DIFFERENT\n");
   }

   return( 0);
}
