#include <mulle-objc-runtime/mulle-objc-runtime.h>


// no support for the package keyword
@interface SomeString
{
   char  *_s;
   int   _len;
}
@end

SomeString  *a = (SomeString *) @"a";
SomeString  *ab = (SomeString *) @"ab";
SomeString  *abc = (SomeString *) @"abc";
SomeString  *abcd = (SomeString *) @"abcd";
SomeString  *abcde = (SomeString *) @"abcde";
SomeString  *abcdeg = (SomeString *) @"abcdeg";
SomeString  *abcdegi = (SomeString *) @"abcdegi";
SomeString  *abcdegil = (SomeString *) @"abcdegil";
SomeString  *abcdegilm = (SomeString *) @"abcdegilm";
SomeString  *abcdegilmo = (SomeString *) @"abcdegilmo";
SomeString  *abcdegilmop = (SomeString *) @"abcdegilmop";
SomeString  *abcdegilmopr = (SomeString *) @"abcdegilmopr";
SomeString  *abcdegilmoprs = (SomeString *) @"abcdegilmoprs";
SomeString  *abcdegilmoprst = (SomeString *) @"abcdegilmoprst";
SomeString  *abcdegilmoprstu = (SomeString *) @"abcdegilmoprstu";


@implementation SomeString

+ (Class) class
{
   return( self);
}


- (void) print
{
   printf( "[%c] %.*s\n", mulle_objc_taggedpointer_get_index( self) ? '*' : '-', self->_len, self->_s);
}

@end



@implementation SomeString5TaggedPointer : SomeString

// static inline char   *mulle_char5_get_charset( void)
// {
//    static const char  table[] =
//    {
//        0,  '.', 'A', 'C',  'D', 'E', 'I', 'N',
//       'O', 'P', 'S', 'T',  '_', 'a', 'b', 'c',
//       'd', 'e', 'f', 'g',  'h', 'i', 'l', 'm',
//       'n', 'o', 'p', 'r',  's', 't', 'u', 'y',
//       0  // bonus zero for tests :)
//    };
//    return( (char *) table);
// }
//
//
// static inline int   mulle_char5_decode_character( int c)
// {
//    assert( c >= 0 && c < 32);
//
//    return( mulle_char5_get_charset()[ c]);
// }
//
//
size_t  mulle_char5_decode32_ascii( uint32_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;
   int    char5;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      char5 = value & 0x1F;
      *s++  = (char) mulle_char5_decode_character( char5);

      value >>= 5;
   }
   return( s - dst);
}



size_t  mulle_char5_decode64_ascii( uint64_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;
   int    char5;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      char5 = value & 0x1F;
      *s++  = (char) mulle_char5_decode_character( char5);

      value >>= 5;
   }
   return( s - dst);
}


static inline size_t   mulle_char5_decode_ascii( uintptr_t value, char *src, size_t len)
{
 if( sizeof( uintptr_t) == sizeof( uint32_t))
    return( mulle_char5_decode32_ascii( (uint32_t) value, src, len));
 return( mulle_char5_decode64_ascii( value, src, len));
}



- (void) print
{
   uintptr_t   value;
   char        buf[ 16];
   size_t      len;

   value = mulle_objc_taggedpointer_get_unsigned_value( self);
   len   = mulle_char5_decode_ascii( value, buf, sizeof( buf));
   printf( "[%c] %.*s\n", mulle_objc_taggedpointer_get_index( self) ? '*' : '-', (int) len, buf);
}

@end


@implementation SomeString7TaggedPointer : SomeString

size_t  mulle_char7_decode32_ascii( uint32_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      *s++    = value & 0x7F;
      value >>= 7;
   }
   return( s - dst);
}


size_t  mulle_char7_decode64_ascii( uint64_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      *s++    = value & 0x7F;
      value >>= 7;
   }
   return( s - dst);
}


static inline size_t   mulle_char7_decode_ascii( uintptr_t value, char *src, size_t len)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( mulle_char7_decode32_ascii( (uint32_t) value, src, len));
   return( mulle_char7_decode64_ascii( value, src, len));
}


- (void) print
{
   uintptr_t   value;
   char        buf[ 32];
   size_t      len;

   value = mulle_objc_taggedpointer_get_unsigned_value( self);
   len   = mulle_char7_decode_ascii( value, buf, sizeof( buf));
   printf( "[%c] %.*s\n", mulle_objc_taggedpointer_get_index( self) ? '*' : '-', (int) len, buf);
}

@end


int   main( void)
{
   struct _mulle_objc_universe  *universe;
   struct _mulle_objc_class     *cls;

   universe = mulle_objc_global_get_universe( 0);
   cls      = (struct _mulle_objc_class *) [SomeString class];
   _mulle_objc_universe_set_staticstringclass( universe, cls, 1);

   // 0x1 constant must be in "tune" with the compiler
   cls     = (struct _mulle_objc_class *)  [SomeString5TaggedPointer class];
   _mulle_objc_universe_set_taggedpointerclass_at_index( universe, cls, 0x1);

   // 0x4 constant must be in "tune" with the compiler
   if( mulle_objc_get_taggedpointer_mask() > 0x4)
   {
      cls     = (struct _mulle_objc_class *)  [SomeString7TaggedPointer class];
      _mulle_objc_universe_set_taggedpointerclass_at_index( universe, cls, 0x4);
   }

   [@"surely not a tagged pointer because its too long." print];
   [@"." print];
   [a print];
   [ab print];
   [abc print];
   [abcd print];
   [abcde print];
   [abcdeg print];

#if 0 // not 32 bit compatible
   [abcdegi print];
   [abcdegil print];
   [abcdegilm print];
   [abcdegilmo print];
   [abcdegilmop print];
   [abcdegilmopr print];

   [abcdegilmoprs print];
   [abcdegilmoprst print];
   [abcdegilmoprstu print];
#endif
   return( 0);
}
