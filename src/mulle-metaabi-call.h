//
//  mulle_metaabi.h
//  mulle-objc-runtime
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#ifndef mulle_metaabi_call_h__
#define mulle_metaabi_call_h__


// this header is "special", you include it on demand. It's not part of
// mulle-objc-runtime.h

#include "mulle-metaabi.h"

///
///

// #pragma clang diagnostic ignored "-Wobjc-root-class"
// #pragma clang diagnostic ignored "-Wgnu-alignof-expression"
//
// // Can't get rid of:
// // warning: incompatible integer to pointer conversion initializing
// // 'typeof (*(&ptrrval))' (aka 'void *') with an expression of type 'intptr_t'
// // (aka 'long') [-Wint-conversion]
//
// #pragma clang diagnostic ignored "-Wint-conversion"


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


#define _mulle_metaabi_object_call_void_return( obj, sel, ...)                                                 \
do                                                                                                             \
{                                                                                                              \
   if( MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) == 0)                                                               \
      _mulle_metaabi_object_call_0( obj, sel);                                                                 \
   else                                                                                                        \
      if( MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) == 1  &&                                                         \
          mulle_metaabi_is_voidptr_compatible_expression( MULLE_C_VA_ARGS_0_WITH_DEFAULT( NULL, __VA_ARGS__))  \
        )                                                                                                      \
         _mulle_metaabi_object_call_1_voidptr( obj, sel __VA_OPT__(,)  __VA_ARGS__);                           \
      else                                                                                                     \
         _mulle_metaabi_object_call_struct_n_void_return( obj, sel __VA_OPT__(,)  __VA_ARGS__);                \
}                                                                                                              \
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
#define _mulle_metaabi_object_call_0_struct_return( p_rval, obj, sel, ...)                   \
do                                                                                           \
{                                                                                            \
   mulle_metaabi_union_void_parameter(                                                       \
      struct                                                                                 \
      {                                                                                      \
         MULLE_METAABI_STRUCT_FIELD( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)), v)    \
      })                                                                                     \
      param;                                                                                 \
                                                                                             \
   mulle_objc_object_call( obj, sel, &param);                                                \
   *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) = param.r.v;                             \
}                                                                                            \
while( 0)


#define _mulle_metaabi_object_call_struct_n_voidptr_return( p_rval, obj, sel, ...)                              \
do                                                                                                              \
{                                                                                                               \
   void   *rval;                                                                                                \
   mulle_metaabi_union(                                                                                         \
      struct                                                                                                    \
      {                                                                                                         \
         MULLE_METAABI_STRUCT_FIELD( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)), v)                       \
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
   *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) =                                                           \
      (__typeof__(*(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval))))                                            \
      {                                                                                                         \
                                                                                                                \
         mulle_metaabi_zero_fp( *(MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)), (intptr_t) rval)              \
      };                                                                                                        \
}                                                                                                               \
while( 0)



#define _mulle_metaabi_object_call_struct_n( p_rval, obj, sel, ...)                                             \
do                                                                                                              \
{                                                                                                               \
   intptr_t dummy;                                                                                               \
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



#define mulle_metaabi_object_call( p_rval, obj, sel, ...)                                                         \
do                                                                                                                \
{                                                                                                                 \
   intptr_t dummy;                                                                                                 \
                                                                                                                  \
   if( MULLE_C_IS_EMPTY( p_rval))                                                                                 \
      _mulle_metaabi_object_call_void_return( obj, sel __VA_OPT__(,) __VA_ARGS__);                                \
   else                                                                                                           \
      if( mulle_metaabi_is_voidptr_compatible_expression( *MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)) &&      \
          MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) <= 1 &&                                                             \
          mulle_metaabi_is_voidptr_compatible_expression( MULLE_C_VA_ARGS_0_WITH_DEFAULT( obj, __VA_ARGS__))      \
        )                                                                                                         \
         _mulle_metaabi_object_call_voidptr( MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),                       \
                                             obj,                                                                 \
                                             sel __VA_OPT__( ,) __VA_ARGS__);                                     \
      else                                                                                                        \
         if( MULLE_C_VA_ARGS_COUNT( __VA_ARGS__) == 0)                                                            \
            _mulle_metaabi_object_call_0_struct_return( MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),            \
                                                        obj,                                                      \
                                                        sel);                                                     \
         else                                                                                                     \
            if( mulle_metaabi_is_voidptr_compatible_expression( *MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval)))  \
                                                                                                                  \
               _mulle_metaabi_object_call_struct_n_voidptr_return(                                                \
                                                    MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),                \
                                                    obj,                                                          \
                                                    sel __VA_OPT__( ,) __VA_ARGS__);                              \
            else                                                                                                  \
               _mulle_metaabi_object_call_struct_n( MULLE_C_VA_ARGS_WITH_DEFAULT( &dummy, p_rval),                \
                                                    obj,                                                          \
                                                    sel __VA_OPT__( ,) __VA_ARGS__);                              \
}                                                                                                                 \
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


#endif

