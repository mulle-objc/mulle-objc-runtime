#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-objc-runtime/mulle-metaabi-call.h>
#include <stdio.h>
#include <string.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wgnu-alignof-expression"

// Can't get rid of:
// warning: incompatible integer to pointer conversion initializing
// 'typeof (*(&ptrrval))' (aka 'void *') with an expression of type 'intptr_t'
// (aka 'long') [-Wint-conversion]

#pragma clang diagnostic ignored "-Wint-conversion"



#define TINY_STRUCT



@interface A @end

@implementation A

+ (Class) class
{
   return( self);
}


// this doesn't work yet
struct tiny
{
   char     a[ 3];
};



struct abc
{
   char     a;
   double   b;
   int      c;
};


+ (void) returnVoid
{
   mulle_printf( "%s\n", __FUNCTION__);
}

+ (void) returnVoidWithInt:(int) v
{
   mulle_printf( "%s (%d)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithFloat:(float) v
{
   mulle_printf( "%s (%g)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithDouble:(double) v
{
   mulle_printf( "%s (%g)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)\n", __FUNCTION__, v.a, v.b, v.c);
}

#ifdef TINY_STRUCT
+ (void) returnVoidWithTinyStruct:(struct tiny) v
{
   struct tiny   tmp = { 'x', 'x', 'x' };

   mulle_metaabi_get_voidptr_parameter( &tmp, _param);
   mulle_printf( "%s ('%c' '%c' '%c')\n", __FUNCTION__, tmp.a[ 0], tmp.a[ 1], tmp.a[ 2]);
}
#endif

+ (void) returnVoidWithChar:(char) a double:(double) b int:(int) c
{
#if 1
   char   a_tmp;
   double b_tmp;
   int    c_tmp;

   mulle_metaabi_get_parameter_n( &a_tmp, _param);
   mulle_metaabi_get_parameter_n( &b_tmp, _param, __typeof__( a_tmp));
   mulle_metaabi_get_parameter_n( &c_tmp, _param, __typeof__( a_tmp), __typeof__( b_tmp));

   mulle_printf( "%s ('%c' %g %d)\n", __FUNCTION__, a_tmp, b_tmp, c_tmp);

#else  // default code, which works well with mulle-clang
   mulle_printf( "%s ('%c' %g %d)\n", __FUNCTION__, a, b, c);
#endif
}

+ (void) returnVoidWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")\n");
   mulle_vararg_end( va);
}

// same procedure as above but now returning values

// int

+ (char) returnChar
{
   mulle_printf( "%s", __FUNCTION__);
   return( 'V');
}

+ (char) returnCharWithChar:(char) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( 'f');
}

+ (char) returnCharWithInt:(int) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( 'L');
}

+ (char) returnCharWithLongLong:(long long) v
{
   mulle_printf( "%s (%lld)", __FUNCTION__, v);
   return( ' ');
}

+ (char) returnCharWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 'B');
}

+ (char) returnCharWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 'o');
}

+ (char) returnCharWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return( 'c');
}

+ (char) returnCharWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 'h');
}

+ (char) returnCharWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( 'u');
}

+ (char) returnCharWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return( 'm');
}


// int

+ (int) returnInt
{
   mulle_printf( "%s", __FUNCTION__);
   return(  1848);
}

+ (int) returnIntWithInt:(int) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return(  1847);
}

+ (int) returnIntWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return(  1849);
}

+ (int) returnIntWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return(  1850);
}

+ (int) returnIntWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return(  1851);
}

+ (int) returnIntWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return(  1852);
}

+ (int) returnIntWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return(  1853);
}

+ (int) returnIntWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return(  1854);
}

// long long

+ (long long) returnLongLong
{
   mulle_printf( "%s", __FUNCTION__);
   return( 1848LL);
}

+ (long long) returnLongLongWithChar:(char) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( 1849LL);
}

+ (long long) returnLongLongWithInt:(int) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( 1850LL);
}

+ (long long) returnLongLongWithLongLong:(long long) v
{
   mulle_printf( "%s (%lld)", __FUNCTION__, v);
   return( 1851LL);
}


+ (long long) returnLongLongWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 1852LL);
}

+ (long long) returnLongLongWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 1853LL);
}

+ (long long) returnLongLongWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return( 1854LL);
}

+ (long long) returnLongLongWithStruct:(struct abc) v
{
   mulle_printf( "%s (%c %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 1855LL);
}

+ (long long) returnLongLongWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s (%c %g %d)", __FUNCTION__, a, b, c);
   return( 1856LL);
}


+ (long long) returnLongLongWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return( 1856LL);
}



// void *
+ (void *) returnVoidptr
{
   mulle_printf( "%s", __FUNCTION__);
   return( (void *) 1848);
}

+ (void *) returnVoidptrWithInt:(int) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( (void *) 1847);
}

+ (void *) returnVoidptrWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( (void *) 1849);
}

+ (void *) returnVoidptrWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( (void *) 1850);
}

+ (void *) returnVoidptrWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return( (void *) 1851);
}

+ (void *) returnVoidptrWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( (void *) 1852);
}

+ (void *) returnVoidptrWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( (void *) 1853);
}

+ (void *) returnVoidptrWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return( (void *) 1854);
}


// float


+ (float) returnFloat
{
   mulle_printf( "%s", __FUNCTION__);
   return( 18.48f);
}

+ (float) returnFloatWithInt:(int) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( 18.47f);
}

+ (float) returnFloatWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 18.49f);
}

+ (float) returnFloatWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 18.50f);
}

+ (float) returnFloatWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return( 18.51f);
}

+ (float) returnFloatWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 18.52f);
}

+ (float) returnFloatWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( 18.53f);
}

+ (float) returnFloatWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return( 18.54f);
}


// double

+ (double) returnDouble
{
   mulle_printf( "%s", __FUNCTION__);
   return( 18.48);
}

+ (double) returnDoubleWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 18.49);
}

+ (double) returnDoubleWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( 18.50);
}

+ (double) returnDoubleWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return( 18.51);
}

+ (double) returnDoubleWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 18.52);
}

+ (double) returnDoubleWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( 18.53);
}

+ (double) returnDoubleWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return( 18.54);
}



// struct abc


+ (struct abc) returnStruct
{
   mulle_printf( "%s", __FUNCTION__);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1848 });
}

+ (struct abc) returnStructWithFloat:(float) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1859 });
}

+ (struct abc) returnStructWithDouble:(double) v
{
   mulle_printf( "%s (%g)", __FUNCTION__, v);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1850 });
}

+ (struct abc) returnStructWithVoidptr:(void *) v
{
   mulle_printf( "%s (%p)", __FUNCTION__, v);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1851 });
}

+ (struct abc) returnStructWithStruct:(struct abc) v
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1852 });
}

+ (struct abc) returnStructWithChar:(char) a double:(double) b int:(int) c
{
   mulle_printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1853 });
}

+ (struct abc) returnStructWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   mulle_printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      mulle_printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   mulle_printf( ")");
   mulle_vararg_end( va);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1854 });
}


// struct tiny return (sizeof <= sizeof(void*))

+ (struct tiny) returnTiny
{
   mulle_printf( "%s", __FUNCTION__);
   return( (struct tiny) {{ 'V', 'f', 'L' }});
}

+ (struct tiny) returnTinyWithInt:(int) v
{
   mulle_printf( "%s (%d)", __FUNCTION__, v);
   return( (struct tiny) {{ 'a', 'b', 'c' }});
}

+ (struct tiny) returnTinyWithTiny:(struct tiny) v
{
   mulle_printf( "%s ('%c' '%c' '%c')", __FUNCTION__, v.a[0], v.a[1], v.a[2]);
   return( (struct tiny) {{ 'x', 'y', 'z' }});
}


// int return with tiny struct param (leaf G: voidptr-compat return, tiny struct param)

+ (int) returnIntWithTiny:(struct tiny) v
{
   mulle_printf( "%s ('%c' '%c' '%c')", __FUNCTION__, v.a[0], v.a[1], v.a[2]);
   return( 1855);
}

@end



int   main()
{
   Class        cls;
   struct abc   abcrval = { 0 };
   void         *ptrrval;
   float        fltrval;
   double       dblrval;
   int          intrval;
   char         charrval;
   long long    lnglngrval;

   cls = [A class];

   mulle_metaabi_call( &charrval, cls, @selector( returnChar));
   mulle_printf( " -> '%c'\n", charrval);

   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithChar:), 18);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithInt:), 1848);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithLongLong:), 1848LL);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithFloat:), 18.48f);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithDouble:), 18.48);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> '%c'\n", charrval);
   mulle_metaabi_call( &charrval, cls, @selector( returnCharWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> '%c'\n", charrval);

   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithInt:), 1848);
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnInt));
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithFloat:), 18.48f);
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithDouble:), 18.48);
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> %d\n", intrval);
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> %d\n", intrval);

   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLong));
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithChar:), 18);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithInt:), 1848);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithLongLong:), 1848LL);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithFloat:), 18.48f);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithDouble:), 18.48);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_call( &lnglngrval, cls, @selector( returnLongLongWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> %lld\n", lnglngrval);

   mulle_metaabi_call( , cls, @selector( returnVoidWithInt:), 1848);
   mulle_metaabi_call( , cls, @selector( returnVoid));
   mulle_metaabi_call( , cls, @selector( returnVoidWithFloat:), 18.48f);
   mulle_metaabi_call( , cls, @selector( returnVoidWithDouble:), 18.48);
   mulle_metaabi_call( , cls, @selector( returnVoidWithVoidptr:), (void *) 0x1848);
   mulle_metaabi_call( , cls, @selector( returnVoidWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));

#ifdef TINY_STRUCT
   mulle_metaabi_call( , cls, @selector( returnVoidWithTinyStruct:), ((struct tiny) { {  'V', 'f', 'L' }}));
#endif
   mulle_metaabi_call( , cls, @selector( returnVoidWithChar:double:int:), (char) 'b', 18.48, 1848);

   mulle_metaabi_call( , cls, @selector( returnVoidWithVA:), 4, 1, 8, 4, 8);

   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithInt:), 1848);
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloat));
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithFloat:), 18.48f);
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithDouble:), 18.48);
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> %g\n", fltrval);
   mulle_metaabi_call( &fltrval, cls, @selector( returnFloatWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> %g\n", fltrval);

   mulle_metaabi_call( &dblrval, cls, @selector( returnDouble));
   mulle_printf( " -> %g\n", dblrval);
   mulle_metaabi_call( &dblrval, cls, @selector( returnDoubleWithFloat:), 18.48f);
   mulle_printf( " -> %g\n", dblrval);
   mulle_metaabi_call( &dblrval, cls, @selector( returnDoubleWithDouble:), 18.48);
   mulle_printf( " -> %g\n", dblrval);
   mulle_metaabi_call( &dblrval, cls, @selector( returnDoubleWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> %g\n", dblrval);
   mulle_metaabi_call( &dblrval, cls, @selector( returnDoubleWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> %g\n", dblrval);
   mulle_metaabi_call( &dblrval, cls, @selector( returnDoubleWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> %g\n", dblrval);
   mulle_metaabi_call( &dblrval, cls, @selector( returnDoubleWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> %g\n", dblrval);

   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptr));
   mulle_printf( " -> %p\n", ptrrval);
   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptrWithFloat:), 18.48f);
   mulle_printf( " -> %p\n", ptrrval);
   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptrWithDouble:), 18.48);
   mulle_printf( " -> %p\n", ptrrval);
   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptrWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> %p\n", ptrrval);
   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptrWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> %p\n", ptrrval);
   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptrWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> %p\n", ptrrval);
   mulle_metaabi_call( &ptrrval, cls, @selector( returnVoidptrWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> %p\n", ptrrval);


   mulle_metaabi_call( &abcrval, cls, @selector( returnStruct));
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_call( &abcrval, cls, @selector( returnStructWithFloat:), 18.48f);
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_call( &abcrval, cls, @selector( returnStructWithDouble:), 18.48);
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_call( &abcrval, cls, @selector( returnStructWithVoidptr:), (void *) 0x1848);
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_call( &abcrval, cls, @selector( returnStructWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_call( &abcrval, cls, @selector( returnStructWithChar:double:int:), 'b', 18.48, 1848);
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_call( &abcrval, cls, @selector( returnStructWithVA:), 4, 1, 8, 4, 8);
   mulle_printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);

   // struct tiny return (sizeof <= sizeof(void*), exercises struct-return path for tiny structs)
   {
      struct tiny  tinyrval;

      mulle_metaabi_call( &tinyrval, cls, @selector( returnTiny));
      mulle_printf( " -> '%c' '%c' '%c'\n", tinyrval.a[0], tinyrval.a[1], tinyrval.a[2]);
      mulle_metaabi_call( &tinyrval, cls, @selector( returnTinyWithInt:), 1848);
      mulle_printf( " -> '%c' '%c' '%c'\n", tinyrval.a[0], tinyrval.a[1], tinyrval.a[2]);
      mulle_metaabi_call( &tinyrval, cls, @selector( returnTinyWithTiny:), ((struct tiny) {{ 'A', 'B', 'C' }}));
      mulle_printf( " -> '%c' '%c' '%c'\n", tinyrval.a[0], tinyrval.a[1], tinyrval.a[2]);
   }

   // int return with tiny struct param (leaf G: voidptr return, tiny struct param)
   mulle_metaabi_call( &intrval, cls, @selector( returnIntWithTiny:), ((struct tiny) {{ 'X', 'Y', 'Z' }}));
   mulle_printf( " -> %d\n", intrval);

   return( 0);
}
