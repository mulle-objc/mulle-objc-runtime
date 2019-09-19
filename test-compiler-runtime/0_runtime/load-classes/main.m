#include <mulle-objc-runtime/mulle-objc-runtime.h>


//
// the problem with this test is, that there are various possibilities
// how order can be correct
//

static char   *loaded[ 32];
static int    n_loaded;


void  add_to_loaded( char *s)
{
   assert( n_loaded < 32);
   loaded[ n_loaded++] = s;
}

int   index_of_loaded( char *s)
{
   char  **p;
   char  **sentinel;

   p        = loaded;
   sentinel = &p[ n_loaded];
   while( p < sentinel)
   {
      if( ! strcmp( *p, s))
         return( p - loaded);
      ++p;
   }

   printf( "%s is missing\n",  s);
   return( -1);
}

static char  cBase[]         = "Base";
static char  cFoo1[]         = "Foo1";
static char  cFoo2[]         = "Foo2";
static char  cFoo3[]         = "Foo3";
static char  cProtoClass1[]  = "ProtoClass1";
static char  cProtoClass2[]  = "ProtoClass2";
static char  cRoot[]         = "Root";


static int   was_loaded_before( char *a, char *b)
{
   int  i_a;
   int  i_b;

   i_a = index_of_loaded( a);
   i_b = index_of_loaded( b);

   if( i_a < 0 || i_b < 0)
      return( 0);
   if( i_a >= i_b)
   {
      printf( "%s ordered incorrectly against %s\n", a, b);
      return( 0);
   }
   return( 1);
}


int   main( int argc, char *argv[])
{
   int   rval;

   if( mulle_objc_global_check_universe( __MULLE_OBJC_UNIVERSENAME__) != mulle_objc_universe_is_ok)
      return( 1);

   rval = was_loaded_before( cRoot, cBase)        |
          was_loaded_before( cBase, cFoo1)        |
          was_loaded_before( cBase, cFoo2)        |
          was_loaded_before( cBase, cFoo3)        |
          was_loaded_before( cProtoClass2, cFoo2) |
          was_loaded_before( cProtoClass1, cFoo3) |
          was_loaded_before( cProtoClass2, cFoo3);

   return( ! rval);
}
