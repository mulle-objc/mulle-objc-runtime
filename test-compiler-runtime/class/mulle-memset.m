#include <mulle-objc-runtime/mulle-objc-runtime.h>


//
// The compiler will mess this up and replace our struct copying code with
// memset. It will even do so, if we decompose the loop into void * access only.
// ridiculous, but true. 
// TODO: experiment with optimizer flags
static inline void   mulle_memclr_void_pointer_aligned( void *entry, size_t size)
{
   struct mulle_memclr_void_ptr_8 { void  *v[ 8]; };
   struct mulle_memclr_void_ptr_2 { void  *v[ 2]; };

   size_t                           n_void_ptr_8s;
   struct mulle_memclr_void_ptr_8   *q;
   struct mulle_memclr_void_ptr_8   *q_sentinel;
   size_t                           n_void_ptr_2s;
   struct mulle_memclr_void_ptr_2   *p;
   struct mulle_memclr_void_ptr_2   *p_sentinel;
   unsigned char                    *s;
   unsigned char                    *sentinel;

   assert( (((intptr_t) entry) & (sizeof( void *) - 1)) == 0);

   sentinel      = &((unsigned char *) entry)[ size];

   n_void_ptr_8s = size / sizeof( struct mulle_memclr_void_ptr_8);
   q             = (struct mulle_memclr_void_ptr_8 *) entry;
   q_sentinel    = &q[ n_void_ptr_8s];
   while( q < q_sentinel)
      *q++ = (struct mulle_memclr_void_ptr_8) { 0 };
   size         %= sizeof( struct mulle_memclr_void_ptr_8);

   n_void_ptr_2s = size / sizeof( struct mulle_memclr_void_ptr_2);
   p             = (struct mulle_memclr_void_ptr_2 *) entry;
   p_sentinel    = &p[ n_void_ptr_2s];
   while( p < p_sentinel)
      *p++ = (struct mulle_memclr_void_ptr_2) { 0 };

   s = (unsigned char *) p;
   while( s < sentinel)
      *s++ = 0;
}      


static void  *x[ 32];


MULLE_C_NEVER_INLINE
static void  prepare( size_t size)
{
   memset( x, 0xff, sizeof( x));
   assert( size <= sizeof( x));
}


MULLE_C_NEVER_INLINE
static void  clear( size_t size)
{
   mulle_memclr_void_pointer_aligned( x, size);
}


MULLE_C_NEVER_INLINE
static void  print( size_t size)
{
   mulle_buffer_do( buffer)
   {
      mulle_buffer_hexdump( buffer, x, sizeof( x), 0, mulle_buffer_hexdump_default);
      printf( "%td:\n%s\n\n", size, mulle_buffer_get_string( buffer));
   }
}


MULLE_C_NEVER_INLINE
static void  test( size_t size)
{
   prepare( size);
   clear( size);
   print( size);
}


//
// memo: be sure to compile everything with --no-mulle-test-define and -O3
//       Change MULLE_OBJC_CLASS_REUSE_ALLOC in mulle-objc-class-convenience.h
//       and see differences
//
int   main( void)
{
   size_t   i;

   for( i = 0; i <= sizeof( x); i++)
      test( i);

   return( 0);
}
