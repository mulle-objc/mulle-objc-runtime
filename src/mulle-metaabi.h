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
#ifndef mulle_metaabi_h__
#define mulle_metaabi_h__

// #include "include.h" // need mulle-c11 for stdalign

#include <stdint.h>
#include <stddef.h>
#include <mulle-c11/mulle-c11.h>
#include <mulle-c11/mulle-c11-eval.h>
#include <mulle-vararg/mulle-vararg.h>

// maybe rename to kind ?
enum mulle_metaabi_param
{
   mulle_metaabi_param_error         = -1,
   mulle_metaabi_param_void          = 0,
   mulle_metaabi_param_void_pointer  = 1,
   mulle_metaabi_param_struct        = 2
};

// these defines are compatible to other universes
// but some aren't implemented in code yet
// NOTE: Avoid adding ';' and ',' because of CSV and mulle-objc-list
// MEMO: now part of mulle_metaabi, but probably should be its own header

#define _C_ARY_B        '['
#define _C_ARY_E        ']'
#define _C_ATOM         '%'
#define _C_BFLD         'b'
#define _C_BOOL         'B'
#define _C_CHARPTR      '*'
#define _C_CHR          'c'
#define _C_CLASS        '#'
// #define _C_COMPLEX      'j'   // no support for this
#define _C_DBL          'd'
#define _C_FLT          'f'
#define _C_FUNCTION     '?'  // same as undef. undef meaning "undefined size"
#define _C_ID           '@'
#define _C_INT          'i'
#define _C_LNG          'l'
#define _C_LNG_DBL      'D'
#define _C_LNG_LNG      'q'
#define _C_PTR          '^'
#define _C_SEL          ':'
#define _C_SHT          's'
#define _C_STRUCT_B     '{'
#define _C_STRUCT_E     '}'
#define _C_UCHR         'C'
#define _C_UINT         'I'
#define _C_ULNG         'L'
#define _C_ULNG_LNG     'Q'
#define _C_UNDEF        '?'  // actually @? is a block, ^? is function pointer (so ?)
#define _C_UNION_B      '('
#define _C_UNION_E      ')'
#define _C_USHT         'S'
#define _C_VECTOR       '!'
#define _C_VOID         'v'

// these future defines are incompatible
// % is like @ but it should be copyied instead of retained
// = is like @ but it should not be retained
#define _C_ASSIGN_ID    '='
#define _C_COPY_ID      '~'
#define _C_RETAIN_ID    _C_ID


struct mulle_metaabi_reader
{
   void   *p;
};


struct mulle_metaabi_writer
{
   void   *p;
};


static inline struct mulle_metaabi_reader
   mulle_metaabi_reader_make( void *buf)
{
   struct mulle_metaabi_reader  list;

   list.p = buf;
   return( list);
}


static inline struct mulle_metaabi_writer
   mulle_metaabi_writer_make( void *buf)
{
   struct mulle_metaabi_writer  list;

   list.p = buf;
   return( list);
}


/* if you need to "manually" call a MetaABI function with a _param block
   use mulle_metaabi_union to generate it. DO NOT CALL IT
   `_param` THOUGH (triggers a compiler bug).

   ex.

   mulle_metaabi_union( NSUInteger, NSRange)   param;

   param.p = NSMakeRange( 1, 1);
   mulle_objc_object_call( obj, sel, &param);
   return( param.rval);
*/

#define mulle_metaabi_voidptr5( size)  \
   ( ((size) + sizeof( void *[ 5]) - 1) / sizeof( void *[ 5]) )

#define mulle_metaabi_sizeof_union(  size) \
   ( sizeof( void *[ 5]) * mulle_metaabi_voidptr5( size) )

#define mulle_metaabi_union( rval_type, param_type)                     \
union                                                                   \
{                                                                       \
   rval_type    r;                                                      \
   param_type   p;                                                      \
   void         *space[ 5][ sizeof( rval_type) > sizeof( param_type)    \
                      ?  mulle_metaabi_voidptr5( sizeof( rval_type))    \
                      :  mulle_metaabi_voidptr5( sizeof( param_type))]; \
}

#define mulle_metaabi_union_voidptr_return( param_type) \
   mulle_metaabi_union( void *, param_type)

#define mulle_metaabi_union_voidptr_parameter( return_type) \
   mulle_metaabi_union( return_type, void *)

// quite the same, as we can't define void member. So just ignore
#define mulle_metaabi_union_void_return( param_type) \
   mulle_metaabi_union( void *, param_type)

#define mulle_metaabi_union_void_parameter( return_type) \
   mulle_metaabi_union( return_type, void *)


/*
 * MetaABI struct builder
 *
 * Useful to dynamically build up a metaabi block for interpreters or so.
 * This works fine for regular methods (that expect a struct param).
 *
 * Variadic argument methods (variable arguments, ..., mulle_vararg_list)
 * are more complicated. The initial arguments are passed via metaabi rules.
 * The variable arguments are pushed via mulle_vararg rules. The difference is,
 * that the promotion rules (->int ->double) apply to variable arguments only.
 * This is because the types are not known and can differ on a per call basis
 * (e.g. mulle_msprintf( "%f", d) vs mulle_msprintf( "%d", i);
 */
#define _mulle_metaabi_push( ap, type, value)                      \
do                                                                 \
{                                                                  \
   ap.p             = mulle_pointer_align( ap.p, alignof( type));  \
   *((type *) ap.p) = value;                                       \
   ap.p             = &((char *) ap.p)[ sizeof( type)];            \
}                                                                  \
while( 0)                                                          \


#define mulle_metaabi_push_pointer( ap, value) \
   _mulle_metaabi_push( ap, void *, value)

#define mulle_metaabi_push_functionpointer( ap, value) \
   _mulle_metaabi_push( ap, void (*)( void), value)


#define mulle_metaabi_push_struct( ap, value)           \
do                                                      \
{                                                       \
   ap.p = mulle_pointer_align( ap.p, alignof( value));  \
   memcpy( ap.p, &value, sizeof( value));               \
   ap.p = &((char *) ap.p)[ sizeof( value)];            \
}                                                       \
while( 0)

#define mulle_metaabi_push_union( ap, value)            \
   mulle_metaabi_push_struct( p, value)


#define mulle_metaabi_push_char( ap, value)  \
   _mulle_metaabi_push( ap, char, value)

#define mulle_metaabi_push_short( ap, value)  \
   _mulle_metaabi_push( ap, short, value)

#define mulle_metaabi_push_int( ap, value)  \
   _mulle_metaabi_push( ap, int, value)

#define mulle_metaabi_push_int32( ap, value)  \
   _mulle_metaabi_push( ap, int32_t, value)

#define mulle_metaabi_push_int64( ap, value)  \
   _mulle_metaabi_push( ap, int64_t, value)

#define mulle_metaabi_push_long( ap, value)  \
   _mulle_metaabi_push( ap, long, value)

#define mulle_metaabi_push_longlong( ap, value)  \
   _mulle_metaabi_push( ap, long long, value)


#define mulle_metaabi_push_unsignedchar( ap, value) \
   _mulle_metaabi_push( ap, unsigned char, value)

#define mulle_metaabi_push_unsignedshort( ap, value) \
   _mulle_metaabi_push( ap, unsigned short, value)


#define mulle_metaabi_push_unsignedint( ap, value) \
   _mulle_metaabi_push( ap, unsigned int, value)

#define mulle_metaabi_push_uint32( ap, value)  \
   _mulle_metaabi_push( ap, uint32_t, value)

#define mulle_metaabi_push_uint64( ap, value)  \
   _mulle_metaabi_push( ap, uint64_t, value)

#define mulle_metaabi_push_unsignedlong( ap, value) \
   _mulle_metaabi_push( ap, unsigned long, value)

#define mulle_metaabi_push_unsignedlonglong( ap, value) \
   _mulle_metaabi_push( ap, unsigned long long, value)


#define mulle_metaabi_push_float( ap, value) \
   _mulle_metaabi_push( ap, float, value)

#define mulle_metaabi_push_double( ap, value) \
   _mulle_metaabi_push( ap, double, value)

#define mulle_metaabi_push_longdouble( ap, value) \
   _mulle_metaabi_push( ap, long double, value)


// mulle-vararg look alikes
#define mulle_metaabi_push_integer( ap, type, value) \
   _mulle_metaabi_push( ap, type, (value))


#define mulle_metaabi_push_fp( ap, type, value) \
   _mulle_metaabi_push( ap, type, (value))


/*
 * MetaABI struct reader
 * Useful to dynamically read up a metaabi block return value for interpreters
 */
static inline void  *
   _mulle_metaabi_reader_align_pointer( struct mulle_metaabi_reader *args,
                                        size_t size,
                                        unsigned int align)
{
   char   *q;

   q       = mulle_pointer_align( args->p, align);
   args->p = &q[ size];
   return( q);
}


#define _mulle_metaabi_next( args, type)                                       \
   *(type *) _mulle_metaabi_reader_align_pointer( &args,                       \
                                                  sizeof( type),               \
                                                  alignof( struct{ type x; }))

/*
 * conveniences
 */
#define mulle_metaabi_next_char( ap)  \
   _mulle_metaabi_next( ap, char)

#define mulle_metaabi_next_short( ap)  \
   _mulle_metaabi_next( ap, short)

#define mulle_metaabi_next_int( ap)  \
   _mulle_metaabi_next( ap, int)

#define mulle_metaabi_next_int32( ap)  \
   _mulle_metaabi_next( ap, int32_t)

#define mulle_metaabi_next_int64( ap)  \
   _mulle_metaabi_next( ap, int64_t)

#define mulle_metaabi_next_long( ap)  \
   _mulle_metaabi_next( ap, long)

#define mulle_metaabi_next_longlong( ap)  \
   _mulle_metaabi_next( ap, long long)


#define mulle_metaabi_next_unsignedchar( ap) \
   _mulle_metaabi_next( ap, unsigned char)

#define mulle_metaabi_next_unsignedshort( ap) \
   _mulle_metaabi_next( ap, unsigned short)

#define mulle_metaabi_next_unsignedint( ap) \
   _mulle_metaabi_next( ap, unsigned int)

#define mulle_metaabi_next_uint32( ap)  \
   _mulle_metaabi_next( ap, uint32_t)

#define mulle_metaabi_next_uint64( ap)  \
   _mulle_metaabi_next( ap, uint64_t)

#define mulle_metaabi_next_unsignedlong( ap) \
   _mulle_metaabi_next( ap, unsigned long)

#define mulle_metaabi_next_unsignedlonglong( ap) \
   _mulle_metaabi_next( ap, unsigned long long)


#define mulle_metaabi_next_float( ap) \
   _mulle_metaabi_next( ap, float)

#define mulle_metaabi_next_double( ap) \
   _mulle_metaabi_next( ap, double)

#define mulle_metaabi_next_longdouble( ap) \
   _mulle_metaabi_next( ap, long double)


#define mulle_metaabi_next_pointer( ap, type) \
   _mulle_metaabi_next( ap, type)

#define mulle_metaabi_next_integer( ap, type) \
   _mulle_metaabi_next( ap, type)

#define mulle_metaabi_next_fp( ap, type) \
   _mulle_metaabi_next( ap, type)

#define mulle_metaabi_next_struct( ap, type) \
   _mulle_metaabi_next( ap, type)

#define mulle_metaabi_next_union( ap, type) \
   _mulle_metaabi_next( ap, type)


/*
 * MetaABI introspection. Path to C transpiler support.
 */

// _Generic won't be available everywhere, but this is a define so that
// should not impede compilation at least (will impede usage)
#define mulle_metaabi_is_fp_expression( expr)  \
   _Generic( (expr), float: 1, double: 1, long double: 1, default: 0)

#define mulle_metaabi_is_voidptr_storage_compatible( type_or_expr) \
   (alignof( type_or_expr) <= alignof( void *) && sizeof( type_or_expr) <= sizeof( void *))

#define mulle_metaabi_is_voidptr_compatible_expression( expr) \
   (mulle_metaabi_is_voidptr_storage_compatible( expr) && ! mulle_metaabi_is_fp_expression( expr))


static inline enum mulle_metaabi_param   _mulle_metaabi_get_metaabiparamtype( char *type)
{
   switch( *type)
   {
   case 0            :
   case _C_VOID      : return( mulle_metaabi_param_void);
   case _C_UNDEF     : return( mulle_metaabi_param_error);

#ifdef _C_ATOM
   case _C_ATOM      :
#endif
   case _C_BOOL      :
   case _C_CHARPTR   :
   case _C_RETAIN_ID :
   case _C_COPY_ID   :
   case _C_ASSIGN_ID :
   case _C_CLASS     :
   case _C_SEL       :
   case _C_CHR       :
   case _C_UCHR      :
   case _C_SHT       :
   case _C_USHT      :
   case _C_INT       :
   case _C_UINT      : return( mulle_metaabi_param_void_pointer);

   case _C_LNG       : return( mulle_metaabi_is_voidptr_storage_compatible( long)
                                  ? mulle_metaabi_param_void_pointer
                                  : mulle_metaabi_param_struct);
   case _C_ULNG      : return( mulle_metaabi_is_voidptr_storage_compatible( unsigned long)
                                  ? mulle_metaabi_param_void_pointer
                                  : mulle_metaabi_param_struct);
   case _C_LNG_LNG   : return( mulle_metaabi_is_voidptr_storage_compatible( long long)
                                  ? mulle_metaabi_param_void_pointer
                                  : mulle_metaabi_param_struct);
   case _C_ULNG_LNG  : return( mulle_metaabi_is_voidptr_storage_compatible( unsigned long long)
                                  ? mulle_metaabi_param_void_pointer
                                  : mulle_metaabi_param_struct);
   case _C_PTR       : if( type[ 1] == '?')
                          return( mulle_metaabi_param_void_pointer);
                       return( mulle_metaabi_is_voidptr_storage_compatible( void( *)( void))
                                 ? mulle_metaabi_param_void_pointer
                                 : mulle_metaabi_param_struct);
   default           : return( mulle_metaabi_param_struct);
   }
}

#endif
