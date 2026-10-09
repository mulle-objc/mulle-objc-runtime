#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


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
static int   check( size_t size)
{
   unsigned char   *p;
   size_t          i;

   p = (unsigned char *) x;
   for( i = 0; i < size; i++)
   {
      if( p[ i] != 0x00)
      {
         fprintf( stderr, "byte %td not cleared (size %td)\n",
                         (ptrdiff_t) i, (ptrdiff_t) size);
         return( 1);
      }
   }

   for( ; i < sizeof( x); i++)
   {
      if( p[ i] != 0xff)
      {
         fprintf( stderr, "byte %td cleared beyond size %td\n",
                         (ptrdiff_t) i, (ptrdiff_t) size);
         return( 1);
      }
   }
   return( 0);
}


MULLE_C_NEVER_INLINE
static int   test( size_t size)
{
   prepare( size);
   clear( size);
   return( check( size));
}


//
// memo: be sure to compile everything with --no-mulle-test-define and -O3
//       Change MULLE_OBJC_CLASS_REUSE_ALLOC in mulle-objc-class-convenience.h
//       and see differences
//
int   main( void)
{
   size_t   i;
   int      rval;

   rval = 0;
   for( i = 0; i <= sizeof( x); i++)
      rval |= test( i);

   return( rval ? 1 : 0);
}
