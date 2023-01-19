#ifndef MULLE_PRETTY_CPP_COMPILE
# include <mulle-objc-runtime/mulle-objc-runtime.h>
# include <stdio.h>
# include <string.h>
#else
# include <mulle-objc-runtime/mulle-metaabi.h>
# pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#endif

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wgnu-alignof-expression"

// Can't get rid of:
// warning: incompatible integer to pointer conversion initializing 
// 'typeof (*(&ptrrval))' (aka 'void *') with an expression of type 'intptr_t' 
// (aka 'long') [-Wint-conversion]

#pragma clang diagnostic ignored "-Wint-conversion"



#define TINY_STRUCT  



#define mulle_metaabi_is_int( expr)                                       \
   _Generic( (expr), char: 1, short int: 1, int: 1, long int: 1,          \
                     long long int: 1, unsigned char: 1,                  \
                     unsigned short int: 1, unsigned int: 1,              \
                     unsigned long int: 1, unsigned long long int: 1,     \
                     default: 0)

#define mulle_metaabi_zero_non_int( expr, value)                          \
   _Generic( (expr), char: (value), short int: (value), int: (value),     \
                     long int: (value), long long int: (value),           \
                     unsigned char: (value), unsigned short int: (value), \
                     unsigned int: (value), unsigned long int: (value),   \
                     unsigned long long int: (value),                     \
                     default: 0)

#define mulle_metaabi_zero_fp( expr, value)  \
   _Generic( (expr), float: 0, double: 0, long double: 0, default: (value))



#define MULLE_METAABI_STRUCT_FIELD( expr, s) __typeof__( expr)  s;
#define MULLE_METAABI_STRUCT_VALUE( expr, s) .p.s = (expr)

#define _mulle_metaabi_object_call_struct_n_void_return( obj, sel, ...)                                \
   mulle_objc_object_call( obj, sel,                                                                   \
      & (mulle_metaabi_union_void_return(                                                              \
            struct                                                                                     \
            {                                                                                          \
               MULLE_C_EVAL(                                                                           \
                  MULLE_C_FOR_EACH( MULLE_METAABI_STRUCT_FIELD,                                        \
                                    MULLE_C_VA_ARGS_WITH_DEFAULT( obj, __VA_ARGS__))                   \
               )                                                                                       \
            })                                                                                         \
         )                                                                                             \
         {                                                                                             \
            MULLE_C_EVAL(                                                                              \
               MULLE_C_FOR_EACH_WITH_COMMA_SEPARATOR( MULLE_METAABI_STRUCT_VALUE,                      \
                                                      MULLE_C_VA_ARGS_WITH_DEFAULT( obj, __VA_ARGS__)) \
            )                                                                                          \
         }                                                                                             \
   )

#define _mulle_metaabi_object_call_0( obj, sel) \
   mulle_objc_object_call( obj, sel, obj)

#define _mulle_metaabi_object_call_1_voidptr( obj, sel, ...)                             \
do                                                                                       \
{                                                                                        \
   union                                                                                 \
   {                                                                                     \
      __typeof__( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__))  v;                 \
      void   *param;                                                                     \
   } tmp;                                                                                \
                                                                                         \
   if( sizeof( tmp) > sizeof( void *))                                                   \
      break;                                                                             \
   if( mulle_metaabi_is_int( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__)))         \
      tmp.param = (void *) (intptr_t)                                                    \
         mulle_metaabi_zero_non_int( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__),  \
                                     MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__)); \
   else                                                                                  \
      tmp.v = MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__);                         \
   mulle_objc_object_call( obj, sel, tmp.param);                                         \
}                                                                                        \
while( 0)


#define _mulle_metaabi_object_call_void_return( obj, sel, ...)                                      \
do                                                                                                  \
{                                                                                                   \
   if( MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) == 0)                                                    \
      _mulle_metaabi_object_call_0( obj, sel);                                                      \
   else                                                                                             \
      if( MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) == 1  &&                                              \
          mulle_metaabi_is_voidptr_compatible_expression( MULLE_C_VA_ARGS_0_WITH_DEFAULT( NULL, __VA_ARGS__))  \
        )                                                                                           \
         _mulle_metaabi_object_call_1_voidptr( obj, sel __VA_OPT__(,)  __VA_ARGS__);                \
      else                                                                                          \
         _mulle_metaabi_object_call_struct_n_void_return( obj, sel __VA_OPT__(,)  __VA_ARGS__);     \
}                                                                                                   \
while( 0)


//
// The union is needed for cpp expansion, to fake syntactically correct C
// code, even if the whole exapanded  will be ifed away by the optimizer.
// The break code is just there for compiler warnings suppression, it will
// get optimized away as checks above _mulle_metaabi_object_call_voidptr will
// ensure that only void* or smaller stuff reaches this
//
#define _mulle_metaabi_object_call_voidptr( p_rval, obj, sel, ...)                           \
do                                                                                           \
{                                                                                            \
   void   *param;                                                                            \
   union                                                                                     \
   {                                                                                         \
      __typeof__( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__))  v;                     \
      void   *param;                                                                         \
   } tmp;                                                                                    \
   union                                                                                     \
   {                                                                                         \
      __typeof__( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)))  v;                      \
      void   *ptr;                                                                           \
   } rval;                                                                                   \
                                                                                             \
   if( sizeof( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval))) > sizeof( void *))          \
      break;                                                                                 \
   if( sizeof( tmp) > sizeof( void *))                                                       \
      break;                                                                                 \
   if( mulle_metaabi_is_int( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__)))             \
      tmp.param = (void *) (intptr_t)                                                        \
         mulle_metaabi_zero_non_int( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__),      \
                                     MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__));     \
   else                                                                                      \
      tmp.v = MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__);                             \
                                                                                             \
   rval.ptr = mulle_objc_object_call( obj, sel, tmp.param);                                  \
   if( mulle_metaabi_is_int( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval))))              \
      *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) =                                     \
         (__typeof__( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval))))                     \
         {                                                                                   \
            mulle_metaabi_zero_non_int( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)),    \
                                        (intptr_t) rval.ptr)                                 \
         };                                                                                  \
   else                                                                                      \
      memcpy( p_rval, &rval.v, sizeof( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval))));   \
}                                                                                            \
while( 0)

//
// TODO: as a convenience mulle_objc_object_call with struct return should
//       return _param
//
#define _mulle_metaabi_object_call_0_struct_return( p_rval, obj, sel, ...)                \
do                                                                                        \
{                                                                                         \
   mulle_metaabi_union_void_parameter(                                                    \
      struct                                                                              \
      {                                                                                   \
         MULLE_METAABI_STRUCT_FIELD( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)), v) \
      })                                                                                  \
      param;                                                                              \
                                                                                          \
   mulle_objc_object_call( obj, sel, &param);                                             \
   *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) = param.r.v;                          \
}                                                                                         \
while( 0)


#define _mulle_metaabi_object_call_struct_n_voidptr_return( p_rval, obj, sel, ...)         \
do                                                                                         \
{                                                                                          \
   void   *rval;                                                                           \
   mulle_metaabi_union(                                                                    \
      struct                                                                               \
      {                                                                                    \
         MULLE_METAABI_STRUCT_FIELD( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)), v)  \
      },                                                                                   \
      struct                                                                               \
      {                                                                                    \
         MULLE_C_EVAL( MULLE_C_FOR_EACH( MULLE_METAABI_STRUCT_FIELD,                       \
                                         MULLE_C_VA_ARGS_WITH_DEFAULT( obj, __VA_ARGS__))) \
      })                                                                                   \
      param =                                                                              \
      {                                                                                    \
         MULLE_C_EVAL( MULLE_C_FOR_EACH_WITH_COMMA_SEPARATOR( MULLE_METAABI_STRUCT_VALUE,                       \
                                                              MULLE_C_VA_ARGS_WITH_DEFAULT( obj, __VA_ARGS__))) \
      };                                                                                   \
                                                                                           \
   rval = mulle_objc_object_call( obj, sel, &param);                                       \
   *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) =                                      \
      (__typeof__(*(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval))))                       \
      {                                                                                    \
                                                                                           \
         mulle_metaabi_zero_fp( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)), (intptr_t) rval)  \
      };                                                                                   \
}                                                                                          \
while( 0)



#define _mulle_metaabi_object_call_struct_n( p_rval, obj, sel, ...)                                             \
do                                                                                                              \
{                                                                                                               \
   void   *dummy;                                                                                               \
   void   *rval;                                                                                                \
   mulle_metaabi_union(                                                                                         \
      struct                                                                                                    \
      {                                                                                                         \
         MULLE_METAABI_STRUCT_FIELD( *MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval), v)                         \
      },                                                                                                        \
      struct                                                                                                    \
      {                                                                                                         \
         MULLE_C_EVAL( MULLE_C_FOR_EACH( MULLE_METAABI_STRUCT_FIELD,                                            \
                                         MULLE_C_VA_ARGS_WITH_DEFAULT( obj, __VA_ARGS__)))                      \
      })                                                                                                        \
      param =                                                                                                   \
      {                                                                                                         \
         MULLE_C_EVAL( MULLE_C_FOR_EACH_WITH_COMMA_SEPARATOR( MULLE_METAABI_STRUCT_VALUE,                       \
                                                              MULLE_C_VA_ARGS_WITH_DEFAULT( obj, __VA_ARGS__))) \
      };                                                                                                        \
                                                                                                                \
   rval = mulle_objc_object_call( obj, sel, &param);                                                            \
   *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) = param.r.v;                                                \
}                                                                                                               \
while( 0)



#define mulle_metaabi_object_call( p_rval, obj, sel, ...)                                              \
do                                                                                                     \
{                                                                                                      \
   void   *dummy;                                                                                      \
                                                                                                       \
   if( MULLE_C_IS_EMPTY( p_rval))                                                                      \
      _mulle_metaabi_object_call_void_return( obj, sel __VA_OPT__(,) __VA_ARGS__);                     \
   else                                                                                                \
      if( mulle_metaabi_is_voidptr_compatible_expression( *MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) &&      \
          MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) <= 1 &&                                                  \
          mulle_metaabi_is_voidptr_compatible_expression( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__))      \
        )                                                                                              \
         _mulle_metaabi_object_call_voidptr( MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),            \
                                             obj,                                                      \
                                             sel __VA_OPT__( ,) __VA_ARGS__);                          \
      else                                                                                             \
         if( MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) == 0)                                                 \
            _mulle_metaabi_object_call_0_struct_return( MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval), \
                                                        obj,                                           \
                                                        sel);                                          \
         else                                                                                          \
            if( mulle_metaabi_is_voidptr_compatible_expression( *MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)))  \
                                                                                                       \
               _mulle_metaabi_object_call_struct_n_voidptr_return(                                     \
                                                    MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),     \
                                                    obj,                                               \
                                                    sel __VA_OPT__( ,) __VA_ARGS__);                   \
            else                                                                                       \
               _mulle_metaabi_object_call_struct_n( MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),     \
                                                    obj,                                               \
                                                    sel __VA_OPT__( ,) __VA_ARGS__);                   \
}                                                                                                      \
while( 0)


#define mulle_metaabi_param_struct( ...)                                                      \
   struct                                                                                     \
   {                                                                                          \
      MULLE_C_EVAL( MULLE_C_FOR_EACH( MULLE_METAABI_STRUCT_FIELD __VA_OPT__( ,) __VA_ARGS__)) \
   }                                                                                      

#define MULLE_C_WHEN_NOT( c)         MULLE_C_IIF(c)( MULLE_C_EAT, MULLE_C_EXPAND)

#define _MULLE_C_FOR_EACH_IDENTIFIER( s, first, ...) \
    MULLE_C_WHEN( MULLE_C_HAS_ARGS(__VA_ARGS__)) \
    ( \
        MULLE_C_OBSTRUCT( __MULLE_C_FOR_EACH_IDENTIFIER) () \
        ( \
            s ## i, __VA_ARGS__ \
        ) \
    ) \
    MULLE_C_WHEN_NOT( MULLE_C_HAS_ARGS(__VA_ARGS__)) \
    ( \
      s \
    )

#define __MULLE_C_FOR_EACH_IDENTIFIER() \
    _MULLE_C_FOR_EACH_IDENTIFIER

#define MULLE_C_FOR_EACH_IDENTIFIER( first, ...) \
    _MULLE_C_FOR_EACH_IDENTIFIER( v, first __VA_OPT__(,) __VA_ARGS__)


// give in varargs the parameter types to skip
#define mulle_metaabi_get_parameter_n( p_value, _param, ...)                                                 \
   (*(p_value) = ((mulle_metaabi_param_struct( __VA_ARGS__ __VA_OPT__( ,) __typeof__( *p_value)) *) _param)  \
      ->MULLE_C_EVAL( MULLE_C_FOR_EACH_IDENTIFIER( __VA_ARGS__ __VA_OPT__( ,) __typeof__( *p_value))))                    


// this function returns the arguments as a struct or a void ptr ?
// mulle_metaabi_get_arguments( _param, float, int, )
#define mulle_metaabi_get_voidptr_parameter( p_value, _param)     \
do                                                                \
{                                                                 \
   if( mulle_metaabi_is_voidptr_compatible_expression( *p_value)) \
   {                                                              \
      if( mulle_metaabi_is_int( *p_value))                        \
         *p_value = (__typeof__( *p_value))                       \
         {                                                        \
            mulle_metaabi_zero_non_int( *p_value,                 \
                                        (intptr_t) _param)        \
         };                                                       \
      else                                                        \
      {                                                           \
         memcpy( p_value, &_param, sizeof( *p_value));            \
      }                                                           \
   }                                                              \
   else                                                           \
   {                                                              \
      mulle_metaabi_get_parameter_n( p_value, _param);            \
   }                                                              \
}                                                                 \
while( 0)
//
// feed it with all types before and including wanted parameter
// but varargs can not be zero (and must not be void pointer compatible)
//


// don't call for rval "void"
#define mulle_metaabi_return( rval, _param)                       \
do                                                                \
{                                                                 \
   void                *dummy;                                    \
   __typeof__( rval)   tmp;                                       \
                                                                  \
   if( mulle_metaabi_is_voidptr_compatible_expression( rval))     \
   {                                                              \
      if( mulle_metaabi_is_int( rval))                            \
      {                                                           \
         return( (void *)                                         \
                 {                                                \
                     mulle_metaabi_zero_non_int( rval,            \
                                                (intptr_t) rval)  \
                 }                                                \
               );                                                 \
      }                                                           \
      else                                                        \
      {                                                           \
         tmp = (rval);                                            \
         memcpy( &dummy, &tmp, sizeof( tmp));                     \
         return( dummy);                                          \
      }                                                           \
   }                                                              \
   else                                                           \
   {                                                              \
      ((struct{ __typeof__( rval)  r; } *) _param)->r = rval;     \
      return( (void *) _param);                                   \
   }                                                              \
}                                                                 \
while( 0)




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
   printf( "%s\n", __FUNCTION__);
}

+ (void) returnVoidWithInt:(int) v
{
   printf( "%s (%d)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithFloat:(float) v
{
   printf( "%s (%g)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithDouble:(double) v
{
   printf( "%s (%g)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithVoidptr:(void *) v
{
   printf( "%s (%p)\n", __FUNCTION__, v);
}

+ (void) returnVoidWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)\n", __FUNCTION__, v.a, v.b, v.c);
}

#ifdef TINY_STRUCT
+ (void) returnVoidWithTinyStruct:(struct tiny) v
{
   struct tiny   tmp = { 'x', 'x', 'x' };

   mulle_metaabi_get_voidptr_parameter( &tmp, _param);
   printf( "%s ('%c' '%c' '%c')\n", __FUNCTION__, tmp.a[ 0], tmp.a[ 1], tmp.a[ 2]);
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

   printf( "%s ('%c' %g %d)\n", __FUNCTION__, a_tmp, b_tmp, c_tmp);

#else  // default code, which works well with mulle-clang
   printf( "%s ('%c' %g %d)\n", __FUNCTION__, a, b, c);
#endif   
}

+ (void) returnVoidWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")\n");
   mulle_vararg_end( va);
}

// same procedure as above but now returning values

// int

+ (char) returnChar
{
   printf( "%s", __FUNCTION__);
   return( 'V');
}

+ (char) returnCharWithChar:(char) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return( 'f');
}

+ (char) returnCharWithInt:(int) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return( 'L');
}

+ (char) returnCharWithLongLong:(long long) v
{
   printf( "%s (%lld)", __FUNCTION__, v);
   return( ' ');
}

+ (char) returnCharWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 'B');
}

+ (char) returnCharWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 'o');
}

+ (char) returnCharWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
   return( 'c');
}

+ (char) returnCharWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 'h');
}

+ (char) returnCharWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( 'u');
}

+ (char) returnCharWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return( 'm');
}


// int

+ (int) returnInt
{
   printf( "%s", __FUNCTION__);
   return(  1848);
}

+ (int) returnIntWithInt:(int) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return(  1847);
}

+ (int) returnIntWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return(  1849);
}

+ (int) returnIntWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return(  1850);
}

+ (int) returnIntWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
   return(  1851);
}

+ (int) returnIntWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return(  1852);
}

+ (int) returnIntWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return(  1853);
}

+ (int) returnIntWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return(  1854);
}

// long long

+ (long long) returnLongLong
{
   printf( "%s", __FUNCTION__);
   return( 1848LL);
}

+ (long long) returnLongLongWithChar:(char) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return( 1849LL);
}

+ (long long) returnLongLongWithInt:(int) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return( 1849LL);
}

+ (long long) returnLongLongWithLongLong:(long long) v
{
   printf( "%s (%lld)", __FUNCTION__, v);
   return( 1850LL);
}


+ (long long) returnLongLongWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 1851LL);
}

+ (long long) returnLongLongWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 1852LL);
}

+ (long long) returnLongLongWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
#if 1
   // this is not good. v is not necessarily inside the _param block
   // because it could be copied outside to a stack position. _param
   // should be available here, but the compiler doesn't give, which 
   // is crazy bug!
   mulle_metaabi_return( 1853LL, (void *) &v);
#else   
   return( 1853LL);
#endif   
}

+ (long long) returnLongLongWithStruct:(struct abc) v
{
   printf( "%s (%c %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 1854LL);
}

+ (long long) returnLongLongWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s (%c %g %d)", __FUNCTION__, a, b, c);
   return( 1855LL);
}


+ (long long) returnLongLongWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return( 1856LL);
}



// void *
+ (void *) returnVoidptr
{
   printf( "%s", __FUNCTION__);
   return( (void *) 1848);
}

+ (void *) returnVoidptrWithInt:(int) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return( (void *) 1847);
}

+ (void *) returnVoidptrWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( (void *) 1849);
}

+ (void *) returnVoidptrWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( (void *) 1850);
}

+ (void *) returnVoidptrWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
   return( (void *) 1851);
}

+ (void *) returnVoidptrWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( (void *) 1852);
}

+ (void *) returnVoidptrWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( (void *) 1853);
}

+ (void *) returnVoidptrWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return( (void *) 1854);
}


// float


+ (float) returnFloat
{
   printf( "%s", __FUNCTION__);
   return( 18.48f);
}

+ (float) returnFloatWithInt:(int) v
{
   printf( "%s (%d)", __FUNCTION__, v);
   return( 18.47f);
}

+ (float) returnFloatWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 18.49f);
}

+ (float) returnFloatWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 18.50f);
}

+ (float) returnFloatWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
   return( 18.51f);
}

+ (float) returnFloatWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 18.52f);
}

+ (float) returnFloatWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( 18.53f);
}

+ (float) returnFloatWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return( 18.54f);
}


// double

+ (double) returnDouble
{
   printf( "%s", __FUNCTION__);
   return( 18.48);
}

+ (double) returnDoubleWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 18.49);
}

+ (double) returnDoubleWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( 18.50);
}

+ (double) returnDoubleWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
   return( 18.51);
}

+ (double) returnDoubleWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( 18.52);
}

+ (double) returnDoubleWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( 18.53);
}

+ (double) returnDoubleWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return( 18.54);
}



// struct abc


+ (struct abc) returnStruct
{
   printf( "%s", __FUNCTION__);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1848 });
}

+ (struct abc) returnStructWithFloat:(float) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1859 });
}

+ (struct abc) returnStructWithDouble:(double) v
{
   printf( "%s (%g)", __FUNCTION__, v);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1850 });
}

+ (struct abc) returnStructWithVoidptr:(void *) v
{
   printf( "%s (%p)", __FUNCTION__, v);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1851 });
}

+ (struct abc) returnStructWithStruct:(struct abc) v
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, v.a, v.b, v.c);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1852 });
}

+ (struct abc) returnStructWithChar:(char) a double:(double) b int:(int) c
{
   printf( "%s ('%c' %g %d)", __FUNCTION__, a, b, c);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1853 });
}

+ (struct abc) returnStructWithVA:(int) n, ...
{
   mulle_vararg_list   va;

   printf( "%s (%d ", __FUNCTION__, n);
   mulle_vararg_start( va, n);
   while( n)
   {
      printf( ", %d", mulle_vararg_next_int( va));
      --n;
   }
   printf( ")");
   mulle_vararg_end( va);
   return( (struct abc) { .a = 'a', .b = 18.48, .c = 1854 });
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

//   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStruct));
//   printf( "%c %g %d\n", abcrval.a, abcrval.b, abcrval.c);
//   memset( &abcrval, 0, sizeof( abcrval));

#if 0
#ifdef MULLE_PRETTY_CPP_COMPILE
   _mulle_metaabi_object_call_void_return( cls, @selector( returnStructWithChar:double:int:));
   _mulle_metaabi_object_call_void_return( cls, @selector( returnStructWithChar:double:int:), 18.48f);
   _mulle_metaabi_object_call_void_return( cls, @selector( returnStructWithChar:double:int:), (void *) 0x1848);
   _mulle_metaabi_object_call_void_return( cls, @selector( returnStructWithChar:double:int:), 'b', 18.48, 1848);

   _mulle_metaabi_object_call_0_struct_return( &abcrval, cls, @selector( returnStructWithChar:double:int:));
   _mulle_metaabi_object_call_0_struct_return( &abcrval, cls, @selector( returnStructWithChar:double:int:), 18.48f);
   _mulle_metaabi_object_call_0_struct_return( &abcrval, cls, @selector( returnStructWithChar:double:int:), (void *) 0x1848);
   _mulle_metaabi_object_call_0_struct_return( &abcrval, cls, @selector( returnStructWithChar:double:int:), 'b', 18.48, 1848);

   _mulle_metaabi_object_call_struct_n( &abcrval, cls, @selector( returnStructWithChar:double:int:));
   _mulle_metaabi_object_call_struct_n( &abcrval, cls, @selector( returnStructWithChar:double:int:), 18.48f);
   _mulle_metaabi_object_call_struct_n( &abcrval, cls, @selector( returnStructWithChar:double:int:), (void *) 0x1848);
   _mulle_metaabi_object_call_struct_n( &abcrval, cls, @selector( returnStructWithChar:double:int:), 'b', 18.48, 1848);

   _mulle_metaabi_object_call_voidptr( &ptrrval, cls, @selector( returnStructWithChar:double:int:));
   _mulle_metaabi_object_call_voidptr( &ptrrval, cls, @selector( returnStructWithChar:double:int:), 18.48f);
   _mulle_metaabi_object_call_voidptr( &ptrrval, cls, @selector( returnStructWithChar:double:int:), (void *) 0x1848);
   _mulle_metaabi_object_call_voidptr( &ptrrval, cls, @selector( returnStructWithChar:double:int:), 'b', 18.48, 1848);
#endif
#endif

#if 1
   mulle_metaabi_object_call( &charrval, cls, @selector( returnChar));
   printf( " -> '%c'\n", charrval);
 
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithChar:), 18);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithInt:), 1848);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithLongLong:), 1848LL);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithFloat:), 18.48f);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithDouble:), 18.48);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithVoidptr:), (void *) 0x1848);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> '%c'\n", charrval);
   mulle_metaabi_object_call( &charrval, cls, @selector( returnCharWithVA:), 4, 1, 8, 4, 8);
   printf( " -> '%c'\n", charrval);

   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithInt:), 1848);
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnInt));
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithFloat:), 18.48f);
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithDouble:), 18.48);
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithVoidptr:), (void *) 0x1848);
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> %d\n", intrval);
   mulle_metaabi_object_call( &intrval, cls, @selector( returnIntWithVA:), 4, 1, 8, 4, 8);
   printf( " -> %d\n", intrval);

   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLong));
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithChar:), 18);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithInt:), 1848);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithLongLong:), 1848LL);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithFloat:), 18.48f);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithDouble:), 18.48);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithVoidptr:), (void *) 0x1848);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> %lld\n", lnglngrval);
   mulle_metaabi_object_call( &lnglngrval, cls, @selector( returnLongLongWithVA:), 4, 1, 8, 4, 8);
   printf( " -> %lld\n", lnglngrval);

   mulle_metaabi_object_call( , cls, @selector( returnVoidWithInt:), 1848);
   mulle_metaabi_object_call( , cls, @selector( returnVoid));
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithFloat:), 18.48f);
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithDouble:), 18.48);
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithVoidptr:), (void *) 0x1848);
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
#endif

#ifdef TINY_STRUCT   
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithTinyStruct:), ((struct tiny) { {  'V', 'f', 'L' }}));
#endif   
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithChar:double:int:), (char) 'b', 18.48, 1848);
#if 1
   mulle_metaabi_object_call( , cls, @selector( returnVoidWithVA:), 4, 1, 8, 4, 8);

   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithInt:), 1848);
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloat));
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithFloat:), 18.48f);
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithDouble:), 18.48);
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithVoidptr:), (void *) 0x1848);
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> %g\n", fltrval);
   mulle_metaabi_object_call( &fltrval, cls, @selector( returnFloatWithVA:), 4, 1, 8, 4, 8);
   printf( " -> %g\n", fltrval);

   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDouble));
   printf( " -> %g\n", dblrval);
   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDoubleWithFloat:), 18.48f);
   printf( " -> %g\n", dblrval);
   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDoubleWithDouble:), 18.48);
   printf( " -> %g\n", dblrval);
   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDoubleWithVoidptr:), (void *) 0x1848);
   printf( " -> %g\n", dblrval);
   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDoubleWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> %g\n", dblrval);
   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDoubleWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> %g\n", dblrval);
   mulle_metaabi_object_call( &dblrval, cls, @selector( returnDoubleWithVA:), 4, 1, 8, 4, 8);
   printf( " -> %g\n", dblrval);

   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptr));
   printf( " -> %p\n", ptrrval);
   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptrWithFloat:), 18.48f);
   printf( " -> %p\n", ptrrval);
   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptrWithDouble:), 18.48);
   printf( " -> %p\n", ptrrval);
   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptrWithVoidptr:), (void *) 0x1848);
   printf( " -> %p\n", ptrrval);
   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptrWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> %p\n", ptrrval);
   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptrWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> %p\n", ptrrval);
   mulle_metaabi_object_call( &ptrrval, cls, @selector( returnVoidptrWithVA:), 4, 1, 8, 4, 8);
   printf( " -> %p\n", ptrrval);


   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStruct));
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStructWithFloat:), 18.48f);
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStructWithDouble:), 18.48);
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStructWithVoidptr:), (void *) 0x1848);
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStructWithStruct:), ((struct abc) { 'b', 18.48, 1848 }));
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStructWithChar:double:int:), 'b', 18.48, 1848);
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
   mulle_metaabi_object_call( &abcrval, cls, @selector( returnStructWithVA:), 4, 1, 8, 4, 8);
   printf( " -> a='%c' b=%g c=%d\n", abcrval.a, abcrval.b, abcrval.c);
#endif

   return( 0);
}
