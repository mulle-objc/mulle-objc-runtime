#import <mulle-objc-runtime/mulle-objc-runtime.h>


@interface A

+ (struct _mulle_objc_class *) class;

- (void) a;

@end

@interface B : A

- (void) b;

@end


@interface A( X)

- (void) a_x;

@end

@interface B( X)

- (void) b_x;

@end


@interface C : A

- (void) c;

@end


@implementation A

+ (struct _mulle_objc_class *) class
{
   return( _mulle_objc_infraclass_as_class( (struct _mulle_objc_infraclass *) self));
}


- (void) a
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@implementation B

- (void) b
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation C

+ (void) initialize
{
   struct _mulle_objc_infraclass         *infra;
   struct _mulle_objc_class              *cls;
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_method             *method;

   infra = (struct _mulle_objc_infraclass *) self;
   cls   = _mulle_objc_infraclass_as_class( infra);

   _mulle_objc_searcharguments_init_default( &args, @selector( c));

   method = mulle_objc_class_search_method( cls, &args, 0, NULL);
   method->descriptor.bits |= _mulle_objc_method_preload;
}


- (void) c
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation A (X)

- (void) a_x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@implementation B (X)

- (void) b_x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation C (X)

- (void) c_x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end



static void   test_no_class( struct _mulle_objc_class *cls, char *name)

{
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_method             *method;
   mulle_objc_methodid_t                 sel;

   sel    = mulle_objc_methodid_from_string( name);
   _mulle_objc_searcharguments_init_default( &args, sel);
   method = mulle_objc_class_search_method( cls, &args, MULLE_OBJC_CLASS_DONT_INHERIT_CLASS, NULL);
   printf( "%s %sfound in class %s\n", name, method ? "" : "not ", mulle_objc_class_get_name( cls));
}



int   main()
{
   test_no_class( [A class], "a");
   test_no_class( [A class], "b");
   test_no_class( [A class], "c");
   test_no_class( [A class], "a_x");
   test_no_class( [A class], "b_x");
   test_no_class( [A class], "c_x");

   printf( "\n");

   test_no_class( [B class], "a");
   test_no_class( [B class], "b");
   test_no_class( [B class], "c");
   test_no_class( [B class], "a_x");
   test_no_class( [B class], "b_x");
   test_no_class( [B class], "c_x");

   printf( "\n");

   test_no_class( [C class], "a");
   test_no_class( [C class], "b");
   test_no_class( [C class], "c");
   test_no_class( [C class], "a_x");
   test_no_class( [C class], "b_x");
   test_no_class( [C class], "c_x");

   return( 0);
}
