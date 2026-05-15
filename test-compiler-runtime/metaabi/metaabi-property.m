#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-objc-runtime/mulle-metaabi-call.h>
#include <stdio.h>
#include <string.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wgnu-alignof-expression"
#pragma clang diagnostic ignored "-Wint-conversion"


// tiny struct: 3 bytes, fits in void*
struct tiny
{
   char  a[ 3];
};

// one float in a struct, sizeof == 4, fits in void*
struct one_float
{
   float  x;
};

// two floats: sizeof == 8 == sizeof(void*) on 64-bit
struct two_floats
{
   float  x;
   float  y;
};

// three floats: sizeof == 12 > sizeof(void*), uses struct return
struct three_floats
{
   float  x;
   float  y;
   float  z;
};

// mixed large struct: definitely > sizeof(void*)
struct abc
{
   char     a;
   double   b;
   int      c;
};


@interface A @end

@implementation A

+ (Class) class
{
   return( self);
}


static char                  _charValue;
static int                   _intValue;
static long long             _longLongValue;
static float                 _floatValue;
static double                _doubleValue;
static void                  *_voidptrValue;
static struct tiny           _tinyValue;
static struct one_float      _oneFloatValue;
static struct two_floats     _twoFloatsValue;
static struct three_floats   _threeFloatsValue;
static struct abc            _abcValue;


// char
+ (char) charValue                   { return( _charValue); }
+ (void) setCharValue:(char) v       { _charValue = v; }

// int
+ (int) intValue                     { return( _intValue); }
+ (void) setIntValue:(int) v         { _intValue = v; }

// long long
+ (long long) longLongValue                  { return( _longLongValue); }
+ (void) setLongLongValue:(long long) v      { _longLongValue = v; }

// float
+ (float) floatValue                 { return( _floatValue); }
+ (void) setFloatValue:(float) v     { _floatValue = v; }

// double
+ (double) doubleValue               { return( _doubleValue); }
+ (void) setDoubleValue:(double) v   { _doubleValue = v; }

// void *
+ (void *) voidptrValue              { return( _voidptrValue); }
+ (void) setVoidptrValue:(void *) v  { _voidptrValue = v; }

// struct tiny (3 bytes, fits in void*)
+ (struct tiny) tinyValue                    { return( _tinyValue); }
+ (void) setTinyValue:(struct tiny) v        { _tinyValue = v; }

// struct one_float (4 bytes, fits in void*)
+ (struct one_float) oneFloatValue                   { return( _oneFloatValue); }
+ (void) setOneFloatValue:(struct one_float) v       { _oneFloatValue = v; }

// struct two_floats (8 bytes == sizeof(void*) on 64-bit)
+ (struct two_floats) twoFloatsValue                  { return( _twoFloatsValue); }
+ (void) setTwoFloatsValue:(struct two_floats) v      { _twoFloatsValue = v; }

// struct three_floats (12 bytes > sizeof(void*))
+ (struct three_floats) threeFloatsValue                    { return( _threeFloatsValue); }
+ (void) setThreeFloatsValue:(struct three_floats) v        { _threeFloatsValue = v; }

// struct abc (> sizeof(void*))
+ (struct abc) abcValue                      { return( _abcValue); }
+ (void) setAbcValue:(struct abc) v          { _abcValue = v; }

@end



int   main()
{
   Class   cls;

   cls = [A class];

   // --- char ---
   {
      char  set = 'X', got;
      mulle_metaabi_call( , cls, @selector( setCharValue:), set);
      mulle_metaabi_call( &got, cls, @selector( charValue));
      mulle_printf( "char: '%c' %s\n", got, got == set ? "PASS" : "FAIL");
   }

   // --- int ---
   {
      int  set = 1848, got;
      mulle_metaabi_call( , cls, @selector( setIntValue:), set);
      mulle_metaabi_call( &got, cls, @selector( intValue));
      mulle_printf( "int: %d %s\n", got, got == set ? "PASS" : "FAIL");
   }

   // --- long long ---
   {
      long long  set = 1848184818481848LL, got;
      mulle_metaabi_call( , cls, @selector( setLongLongValue:), set);
      mulle_metaabi_call( &got, cls, @selector( longLongValue));
      mulle_printf( "long long: %lld %s\n", got, got == set ? "PASS" : "FAIL");
   }

   // --- float ---
   {
      float  set = 18.48f, got;
      mulle_metaabi_call( , cls, @selector( setFloatValue:), set);
      mulle_metaabi_call( &got, cls, @selector( floatValue));
      mulle_printf( "float: %g %s\n", got, got == set ? "PASS" : "FAIL");
   }

   // --- double ---
   {
      double  set = 18.48, got;
      mulle_metaabi_call( , cls, @selector( setDoubleValue:), set);
      mulle_metaabi_call( &got, cls, @selector( doubleValue));
      mulle_printf( "double: %g %s\n", got, got == set ? "PASS" : "FAIL");
   }

   // --- void * ---
   {
      void  *set = (void *) 0x1848, *got;
      mulle_metaabi_call( , cls, @selector( setVoidptrValue:), set);
      mulle_metaabi_call( &got, cls, @selector( voidptrValue));
      mulle_printf( "voidptr: %p %s\n", got, got == set ? "PASS" : "FAIL");
   }

   // --- struct tiny (3 bytes, fits in void*) ---
   {
      struct tiny  set = {{ 'V', 'f', 'L' }}, got = {{ 0 }};
      mulle_metaabi_call( , cls, @selector( setTinyValue:), set);
      mulle_metaabi_call( &got, cls, @selector( tinyValue));
      mulle_printf( "tiny: '%c' '%c' '%c' %s\n", got.a[0], got.a[1], got.a[2],
         !memcmp( &got, &set, sizeof( set)) ? "PASS" : "FAIL");
   }

   // --- struct one_float (4 bytes, fits in void*) ---
   {
      struct one_float  set = { 1.5f }, got = { 0 };
      mulle_metaabi_call( , cls, @selector( setOneFloatValue:), set);
      mulle_metaabi_call( &got, cls, @selector( oneFloatValue));
      mulle_printf( "one_float: %g %s\n", got.x,
         !memcmp( &got, &set, sizeof( set)) ? "PASS" : "FAIL");
   }

   // --- struct two_floats (8 bytes == sizeof(void*)) ---
   {
      struct two_floats  set = { 1.5f, 2.5f }, got = { 0 };
      mulle_metaabi_call( , cls, @selector( setTwoFloatsValue:), set);
      mulle_metaabi_call( &got, cls, @selector( twoFloatsValue));
      mulle_printf( "two_floats: %g %g %s\n", got.x, got.y,
         !memcmp( &got, &set, sizeof( set)) ? "PASS" : "FAIL");
   }

   // --- struct three_floats (12 bytes > sizeof(void*)) ---
   {
      struct three_floats  set = { 1.5f, 2.5f, 3.5f }, got = { 0 };
      mulle_metaabi_call( , cls, @selector( setThreeFloatsValue:), set);
      mulle_metaabi_call( &got, cls, @selector( threeFloatsValue));
      mulle_printf( "three_floats: %g %g %g %s\n", got.x, got.y, got.z,
         !memcmp( &got, &set, sizeof( set)) ? "PASS" : "FAIL");
   }

   // --- struct abc (> sizeof(void*)) ---
   {
      struct abc  set = { 'a', 18.48, 1848 }, got = { 0 };
      mulle_metaabi_call( , cls, @selector( setAbcValue:), set);
      mulle_metaabi_call( &got, cls, @selector( abcValue));
      mulle_printf( "abc: '%c' %g %d %s\n", got.a, got.b, got.c,
         !memcmp( &got, &set, sizeof( set)) ? "PASS" : "FAIL");
   }

   return( 0);
}
