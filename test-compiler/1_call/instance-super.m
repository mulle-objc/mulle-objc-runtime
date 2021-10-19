#ifdef __MULLE_OBJC__
# include <mulle-objc-runtime/mulle-objc-runtime.h>
#else
extern void  *class_createInstance( Class cls, long extra);
extern void  object_dispose( id obj);
#endif

#include <stdio.h>


@interface Bar

+ (instancetype) alloc;
- (void) dealloc;

- (void) a:(id) a;
- (void) a:(id) a
         b:(id) b;
@end



@implementation Bar

+ (instancetype) alloc
{
#ifdef __MULLE_OBJC__
   return( mulle_objc_infraclass_alloc_instance( self));
#else
   return( class_createInstance( self, 0));
#endif
}

- (void) dealloc
{
#ifdef __MULLE_OBJC__
   mulle_objc_instance_free( self);
#else
   object_dispose( self);
#endif
}

- (void)  a:(id) a
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


- (void) a:(id) a
         b:(id) b
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


@end


@interface Foo : Bar
@end


@implementation Foo

- (void)  a:(id) a
{
   [super a:a];
}


- (void) a:(id) a
         b:(id) b
{
   [super a:a
          b:b];
}


@end



int   main( void)
{
   Foo  *foo;

   foo = [Foo alloc];

   [Foo a:(id) 0];
   [Foo a:(id) 0 b:(id) 1];

   [foo dealloc];

   return( 0);
}
