//
//  mulle_objc_signature.h
//  mulle-objc-runtime
//
//  Created by Nat! on 28.02.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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
#ifndef mulle_objc_signature_h__
#define mulle_objc_signature_h__

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "include.h"  // for alignment mulle_objc_vararg.hcode

// these defines are compatible to other universes
// but some aren't implemented in code yet
// NOTE: Avoid ';' and ',' because of CSV and mulle-objc-list

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


// Type modifiers are no longer encoded in mulle-objc. They slow things down
// and there is no interest in them. The DO stuff and GC is also obsolete
//
// #define _C_BYCOPY       'O'
// #define _C_BYREF        'R'
// #define _C_CONST        'r'  /*  mulle-c compiler doesn't emit it anymore */
// #define _C_GCINVISIBLE  '|'  /* lol */
// #define _C_IN           'n'
// #define _C_INOUT        'N'
// #define _C_ONEWAY       'V'
// #define _C_OUT          'o'

struct mulle_objc_typeinfo
{
   char       *type;                // not a copy(!) keep your passed in in "types" around, will be past "const"
   char       *pure_type_end;       // if you have "{?=QQ}16", will point just after '}'
   char       *name;                // @"NSString", @"<X>"  will be "NSString" in quotes!

   int32_t    invocation_offset;    // only set if used with the enumerator, useful ONLY for NSInvocation!!

   uint32_t   natural_size;
   uint32_t   bits_size;

   uint16_t   bits_struct_alignment;
   uint16_t   natural_alignment;

   uint16_t   n_members;        // 0, for scalar, n: for union (members), array(len), bitfield(len), struct( members)
   char       has_object;
   char       has_retainable_object;
};


//
// You usually don't call this yourself, always check types first
// if type is invalid or empty, will return NULL and set errno to EINVAL
// put in NULL for p_offset, unless you are the enumerator and are aware that
// the first typeinfo here will be rval...
//
char   *__mulle_objc_signature_supply_next_typeinfo( char *types,
                                                     struct mulle_objc_typeinfo *info,
                                                     unsigned int *p_invocation_offset,
                                                     unsigned int  index);


//
// the signature enumerator enumerates self, _cmd, args...
// and at the end, you can ask _mulle_objc_signatureenumerator_rval to
// get the return value info. If you don't ask for it last, the offset
// will not have been computed properly.
//
// i.e.
// mulle_objc_signature_enumerate(..)
// while( _mulle_objc_signatureenumerator_next(..);
// _mulle_objc_signatureenumerator_rval(..)
// _mulle_objc_signatureenumerator_done(..)
//
struct mulle_objc_signatureenumerator
{
   char                         *types;
   unsigned int                 invocation_offset;
   unsigned int                 i;
   struct mulle_objc_typeinfo   rval;
};



MULLE_C_NONNULL_FIRST
static inline int
   _mulle_objc_signatureenumerator_next( struct mulle_objc_signatureenumerator *rover,
                                         struct mulle_objc_typeinfo *info)
{
   if( ! rover->types)
      return( 0);

   rover->types = __mulle_objc_signature_supply_next_typeinfo( rover->types,
                                                               info,
                                                               &rover->invocation_offset,
                                                               rover->i);
   rover->types = (rover->types && *rover->types) ? rover->types : NULL;
   rover->i++;
   return( 1);
}


static inline struct mulle_objc_signatureenumerator
   mulle_objc_signature_enumerate( char *types)
{
   struct mulle_objc_signatureenumerator  rover;

   rover.types  = (types && *types) ? types : NULL;
   if( ! rover.types)
      return( rover);

   // parse first the incomplete rval
   rover.types =  __mulle_objc_signature_supply_next_typeinfo( rover.types,
                                                               &rover.rval,
                                                               NULL,
                                                               0);
   rover.types = (rover.types && *rover.types) ? rover.types : NULL;

   rover.invocation_offset = 0;  // now start at 0
   rover.i                 = 0;

   return( rover);
}


MULLE_C_NONNULL_FIRST
static inline char *
   _mulle_objc_signatureenumerator_get_type( struct mulle_objc_signatureenumerator *rover)
{
   return( rover->rval.type);
}



MULLE_C_NONNULL_FIRST_SECOND
static inline void
   _mulle_objc_signatureenumerator_rval( struct mulle_objc_signatureenumerator *rover,
                                         struct mulle_objc_typeinfo *info)
{
   rover->rval.invocation_offset = rover->invocation_offset;
   memcpy( info, &rover->rval, sizeof( *info));
}


static inline void
   mulle_objc_signatureenumerator_done( struct mulle_objc_signatureenumerator *rover)
{
}



static inline char   *
   _mulle_objc_signature_supply_next_typeinfo( char *types,
                                               struct mulle_objc_typeinfo *info)
{
   if( ! types || ! *types)
      return( NULL);

   return( __mulle_objc_signature_supply_next_typeinfo( types, info, NULL, 0));
}


//
// you should be able to iterate through all types of a signature with
// while( mulle_objc_signature_supply_next_typeinfo( types, &info))
//
static inline char   *
   mulle_objc_signature_supply_next_typeinfo( char *types,
                                              struct mulle_objc_typeinfo *info)
{
   char           *next;

   if( ! types || ! *types)
      return( NULL);

   next = __mulle_objc_signature_supply_next_typeinfo( types, info, NULL, 0);
   return( next);
}


//
// skip to next type if the types is "@:@@\0"
// the sequence will be ":@@\0" "@@\0" "@\0" "\0" NULL
//
char    *mulle_objc_signature_next_type( char *types);


// kinda the same just a little simpler to interact with
char   *mulle_objc_signature_supply_size_and_alignment( char *type,
                                                        unsigned int *size,
                                                        unsigned int *alignment);

unsigned int    mulle_objc_signature_count_typeinfos( char *types);


enum mulle_objc_metaabiparamtype
{
   mulle_objc_metaabiparamtype_error         = -1,
   mulle_objc_metaabiparamtype_void          = 0,
   mulle_objc_metaabiparamtype_void_pointer  = 1,
   mulle_objc_metaabiparamtype_param         = 2
};

//
// -1: error, 0: void, 1: void *, 2: _param
//
// mulle_objc_signature_get_metaabiparamtype will deduce _param correctly from
// the return type also. It needs the complete signature.
//
enum mulle_objc_metaabiparamtype   mulle_objc_signature_get_metaabiparamtype( char *types);

// this method does not inspect the complete signature! only the return type
enum mulle_objc_metaabiparamtype   mulle_objc_signature_get_metaabireturntype( char *type);


// this method does not inspect the complete signature! only the type
static inline enum mulle_objc_metaabiparamtype
   _mulle_objc_signature_get_metaabiparamtype( char *type)
{
   return( mulle_objc_signature_get_metaabireturntype( type)); // sic(!)
}


//
// not sure of this is really needed or paranoia
// skip prefix fluff of signature
//
char   *_mulle_objc_signature_skip_extendedtypeinfo( char *s);

static inline int  _mulle_objc_signature_pedantic_compare( char *a, char *b)
{
   return( strcmp( a, b));
}

int   _mulle_objc_typeinfo_compare( struct mulle_objc_typeinfo *a,
                                    struct mulle_objc_typeinfo *b);

int  _mulle_objc_signature_compare( char *a, char *b);


// check if type is '@' '~' '=' '#' (or as member array, union, struct member)
int   mulle_objc_signature_contains_object( char *type);

// check if type is '@' '~' (or as member in array, union, struct member)
int   mulle_objc_signature_contains_retainableobject( char *type);


// returns strlen()! not sizeof()
static inline size_t  mulle_objc_get_untypedsignature_length( unsigned int args)
{
   // "@@:[@]*n"
   return( args + 3);
}

// size is sizeof( ) here not len
static inline void   _mulle_objc_sprint_untypedsignature( char *buf, size_t size, unsigned int args)
{
   assert( mulle_objc_get_untypedsignature_length( args) + 1 <= size);

   *buf++ = '@';
   *buf++ = '@';
   *buf++ = ':';

   while( args)
   {
     *buf++ = '@';
     --args;
   }
   *buf = 0;
}


#define mulle_objc_type_fits_voidptr( x)  \
   ((sizeof( x) <= sizeof( void *)) && (alignof( x) <= alignof( void *)))


//
// in usual operation, this evaporates as none of the _C_xxxx type
// qualifiers are actually defined and the mulle-objc 0.17 does not
// emit them anymore
//
static inline char   *_mulle_objc_signature_skip_type_qualifier( char *type)
{
   assert( type);

   for(;;)
   {
      switch( *type)
      {
      default :
         return( type);

#ifdef _C_CONST
      case _C_CONST :
#endif
#ifdef _C_IN
      case _C_IN :
#endif
#ifdef _C_INOUT
      case _C_INOUT :
#endif
#ifdef _C_OUT
      case _C_OUT :
#endif
#ifdef _C_BYCOPY
      case _C_BYCOPY :
#endif
#ifdef _C_BYREF
      case _C_BYREF :
#endif
#ifdef _C_ONEWAY
      case _C_ONEWAY :
#endif
#ifdef _C_GCINVISIBLE
      case _C_GCINVISIBLE :
#endif
         continue;
      }
      ++type;
   }
}


#endif /* defined(__MULLE_OBJC__mulle_objc_signature__) */


// Get type and encoding via compiler:
//
// #include <stdio.h>
//
// #define show_encode( x)   printf( "%s=%s\n", #x, @encode( x))
//
//
// int   main( void)
// {
//    show_encode( void (*)( void));
//    return( 0);
// }
