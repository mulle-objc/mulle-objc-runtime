#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdarg.h>


@class Bar;
@class Baz;


@interface Foo

- (void) void_a;
- (void) void_b:(int) v;
- (void) void_c:(id) v;
- (void) void_d:(Bar *) v;
- (void) void_e:(Baz *) v;
- (void) void_f:(Bar *) v :(Baz *) v;
- (void) void_g:(Baz *) v :(Bar *) v;

- (int) int_a;
- (int) int_b:(int) v;
- (int) int_c:(id) v;
- (int) int_d:(Bar *) v;
- (int) int_e:(Baz *) v;
- (int) int_f:(Bar *) v :(Baz *) v;
- (int) int_g:(Baz *) v :(Bar *) v;

- (id) id_a;
- (id) id_b:(int) v;
- (id) id_c:(id) v;
- (id) id_d:(Bar *) v;
- (id) id_e:(Baz *) v;
- (id) id_f:(Bar *) v :(Baz *) v;
- (id) id_g:(Baz *) v :(Bar *) v;

- (Bar *) bar_a;
- (Bar *) bar_b:(int) v;
- (Bar *) bar_c:(id) v;
- (Bar *) bar_d:(Bar *) v;
- (Bar *) bar_e:(Baz *) v;
- (Bar *) bar_f:(Bar *) v :(Baz *) v;
- (Bar *) bar_g:(Baz *) v :(Bar *) v;

- (Baz *) baz_a;
- (Baz *) baz_b:(int) v;
- (Baz *) baz_c:(id) v;
- (Baz *) baz_d:(Bar *) v;
- (Baz *) baz_e:(Baz *) v;
- (Baz *) baz_f:(Bar *) v :(Baz *) v;
- (Baz *) baz_g:(Baz *) v :(Bar *) v;

@end


@implementation Foo

- (void) void_a {}
- (void) void_b:(int) v {}
- (void) void_c:(id) v {}
- (void) void_d:(Bar *) v {}
- (void) void_e:(Baz *) v {}
- (void) void_f:(Bar *) v :(Baz *) v {}
- (void) void_g:(Baz *) v :(Bar *) v {}

- (int) int_a { return( 0); }
- (int) int_b:(int) v { return( 0); }
- (int) int_c:(id) v { return( 0); }
- (int) int_d:(Bar *) v { return( 0); }
- (int) int_e:(Baz *) v { return( 0); }
- (int) int_f:(Bar *) v :(Baz *) v { return( 0); }
- (int) int_g:(Baz *) v :(Bar *) v { return( 0); }

- (id) id_a { return( 0); }
- (id) id_b:(int) v { return( 0); }
- (id) id_c:(id) v { return( 0); }
- (id) id_d:(Bar *) v { return( 0); }
- (id) id_e:(Baz *) v { return( 0); }
- (id) id_f:(Bar *) v :(Baz *) v { return( 0); }
- (id) id_g:(Baz *) v :(Bar *) v { return( 0); }

- (Bar *) bar_a { return( 0); }
- (Bar *) bar_b:(int) v { return( 0); }
- (Bar *) bar_c:(id) v { return( 0); }
- (Bar *) bar_d:(Bar *) v { return( 0); }
- (Bar *) bar_e:(Baz *) v { return( 0); }
- (Bar *) bar_f:(Bar *) v :(Baz *) v { return( 0); }
- (Bar *) bar_g:(Baz *) v :(Bar *) v { return( 0); }

- (Baz *) baz_a { return( 0); }
- (Baz *) baz_b:(int) v { return( 0); }
- (Baz *) baz_c:(id) v { return( 0); }
- (Baz *) baz_d:(Bar *) v { return( 0); }
- (Baz *) baz_e:(Baz *) v { return( 0); }
- (Baz *) baz_f:(Bar *) v :(Baz *) v { return( 0); }
- (Baz *) baz_g:(Baz *) v :(Bar *) v { return( 0); }

@end


static char   *s_comparison( int rval)
{
   if( ! rval)
      return( "==");
   return( rval < 0 ? "<" : ">");
}



static void  test( SEL a, SEL b)
{
   int                            rval[ 3];
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_infraclass  *infra;
   char                           *a_name;
   char                           *b_name;
   char                           *a_signature;
   char                           *b_signature;
   struct _mulle_objc_method      *a_method;
   struct _mulle_objc_method      *b_method;

   universe    = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);
   infra       = mulle_objc_universe_lookup_infraclass_nofail( universe, @selector( Foo));

   a_method    = mulle_objc_class_defaultsearch_method( _mulle_objc_infraclass_as_class( infra), a);
   a_name      = mulle_objc_method_get_name( a_method);
   a_signature = mulle_objc_method_get_signature( a_method);

   b_method    = mulle_objc_class_defaultsearch_method( _mulle_objc_infraclass_as_class( infra), b);
   b_name      = mulle_objc_method_get_name( b_method);
   b_signature = mulle_objc_method_get_signature( b_method);

   rval[ 0]    = _mulle_objc_signature_compare_lenient( a_signature, b_signature);
   rval[ 1]    = _mulle_objc_signature_compare( a_signature, b_signature);
   rval[ 2]    = _mulle_objc_signature_compare_strict( a_signature, b_signature);

   printf( "%s,%s | compare( %s, %s) | %s,%s,%s\n",
               a_name, b_name,
               a_signature, b_signature,
               s_comparison( rval[ 0]),
               s_comparison( rval[ 1]),
               s_comparison( rval[ 2]));
}


SEL   selectors[] =
{
   @selector( void_a),
   @selector( void_b:),
   @selector( void_c:),
   @selector( void_d:),
   @selector( void_e:),
   @selector( void_f::),
   @selector( void_g::),

   @selector( int_a),
   @selector( int_b:),
   @selector( int_c:),
   @selector( int_d:),
   @selector( int_e:),
   @selector( int_f::),
   @selector( int_g::),

   @selector( bar_a),
   @selector( bar_b:),
   @selector( bar_c:),
   @selector( bar_d:),
   @selector( bar_e:),
   @selector( bar_f::),
   @selector( bar_g::),

   @selector( baz_a),
   @selector( baz_b:),
   @selector( baz_c:),
   @selector( baz_d:),
   @selector( baz_e:),
   @selector( baz_f::),
   @selector( baz_g::)
};


int   main( void)
{
   SEL   *p;
   SEL   *q;
   SEL   *sentinel;

   p        = &selectors[ 0];
   sentinel = &p[ sizeof( selectors) / sizeof( SEL)];

   while( p < sentinel)
   {
      q = &selectors[ 0];
      while( q < sentinel)
      {
         test( *p, *q);
         ++q;
      }
      ++p;
   }
   return( 0);
}
