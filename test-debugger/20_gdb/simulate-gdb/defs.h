#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <mulle-dlfcn/mulle-dlfcn.h>


typedef uint64_t CORE_ADDR;
typedef uint64_t ULONGEST;

struct minimal_symbol;
struct objfile;


struct bound_minimal_symbol
{
  CORE_ADDR   minsym;
};

enum bfd_endian { BFD_ENDIAN_BIG, BFD_ENDIAN_LITTLE, BFD_ENDIAN_UNKNOWN };

#if ! defined(__LITTLE_ENDIAN__) && ! defined(__BIG_ENDIAN__)
# if defined( __BYTE_ORDER__) && defined( __ORDER_LITTLE_ENDIAN__) && defined( __ORDER_LITTLE_ENDIAN__)
#  if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#   define __LITTLE_ENDIAN__  0
#   define __BIG_ENDIAN__     1
#  else
#   define __LITTLE_ENDIAN__  1
#   define __BIG_ENDIAN__     0
#  endif
# else
#  if defined( __LITTE_ENDIAN__) && defined( __BIG_ENDIAN__)
#   error Both __LITTLE_ENDIAN__ and __BIG_ENDIAN__ defined
#  else
#   ifdef _WIN32
#    define __LITTLE_ENDIAN__  1
#    define __BIG_ENDIAN__     0
#   else
#    error Neither __LITTLE_ENDIAN__ nor __BIG_ENDIAN__ defined
#   endif
#  endif
# endif
#endif


static inline CORE_ADDR
   BMSYMBOL_VALUE_ADDRESS( struct bound_minimal_symbol p)
{
   return( p.minsym);
}


struct gdbarch
{
   enum bfd_endian  byteorder;
   int              ptr_bit;
};


static inline void  gdbarch_init( struct gdbarch *arch)
{
#if __BIG_ENDIAN__
   arch->byteorder = BFD_ENDIAN_BIG;
#else
   arch->byteorder = BFD_ENDIAN_LITTLE;
#endif
   arch->ptr_bit   = sizeof( void *) * 8;
}


static inline enum bfd_endian  gdbarch_byte_order( struct gdbarch *arch)
{
   return( arch->byteorder);
}


static inline int  gdbarch_ptr_bit( struct gdbarch *arch)
{
   return( arch->ptr_bit);
}



// possibly overdesigned :) :) :)
static inline ULONGEST
read_memory_unsigned_integer( CORE_ADDR memaddr,
                              int len,
                              enum bfd_endian byte_order)
{
  unsigned char  buf[ sizeof (ULONGEST)];
  int            start;
  int            end;
  int            i;
  int            step;
  ULONGEST       value;

  assert( len >= 1 && len <= sizeof( ULONGEST));

  memcpy( buf, (void *) memaddr, len);
  switch( byte_order)
  {
  case BFD_ENDIAN_BIG :
     start = 0;
     end   = len;
     step  = 1;
     break;

  default :
     start = len - 1;
     end   = -1;
     step  = -1;
     break;
  }

  value = 0;
  i     = start;
  do
  {
      value <<= 8;
      value  += buf[ i];
      i      += step;
  }
  while( i != end);

  return( value);
}


#define gdb_assert   assert

static struct bound_minimal_symbol
lookup_bound_minimal_symbol( const char *name)
{
   struct bound_minimal_symbol   sym;

   sym.minsym = (uintptr_t) dlsym( MULLE_RTLD_DEFAULT, name);
   return( sym);
}


CORE_ADDR
objc_find_implementation_from_class( struct gdbarch *gdbarch,
                                     CORE_ADDR classAddr,
                                     CORE_ADDR sel,
                                     long inheritance,
                                     long classid);
