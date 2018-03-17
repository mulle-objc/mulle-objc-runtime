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
#include "dependencies.h"  // for alignment mulle_objc_vararg.hcode


// these defines are compatible to other universes
// but some aren't implemented in code yet
#define _C_ARY_B        '['
#define _C_ARY_E        ']'
#define _C_ATOM         '%'
#define _C_BFLD         'b'
#define _C_BOOL         'B'
#define _C_BYCOPY       'O'
#define _C_BYREF        'R'
#define _C_CHARPTR      '*'
#define _C_CHR          'c'
#define _C_CLASS        '#'
#define _C_CONST        'r'
#define _C_DBL          'd'
#define _C_FLT          'f'
#define _C_GCINVISIBLE  '!'  /* lol */
#define _C_ID           '@'
#define _C_IN           'n'
#define _C_INOUT        'N'
#define _C_INT          'i'
#define _C_LNG          'l'
#define _C_LNG_DBL      'D'
#define _C_LNG_LNG      'q'
#define _C_ONEWAY       'V'
#define _C_OUT          'o'
#define _C_PTR          '^'
#define _C_SEL          ':'
#define _C_SHT          's'
#define _C_STRUCT_B     '{'
#define _C_STRUCT_E     '}'
#define _C_UCHR         'C'
#define _C_UINT         'I'
#define _C_ULNG         'L'
#define _C_ULNG_LNG     'Q'
#define _C_UNDEF        '?'
#define _C_UNION_B      '('
#define _C_UNION_E      ')'
#define _C_USHT         'S'
#define _C_VECTOR       '!'
#define _C_VOID         'v'

// these future defines are incompatible
// % is like @ but it should be copyied instead of retained
// = is like @ but it should not be retained
#define _C_COPY_ID      '~'
#define _C_ASSIGN_ID    '='
#define _C_RETAIN_ID    _C_ID


struct mulle_objc_typeinfo
{
   char       *type;            // not a copy(!) keep your passed in in "types" around, will be past "const"
   char       *pure_type_end;   // if you have "{?=QQ}16", will point just after '}'
   char       *name;            // @"NSString", @"<X>"  will be "NSString" in quotes!

   int32_t    offset;

   uint32_t   natural_size;
   uint32_t   bits_size;

   uint16_t   bits_struct_alignment;
   uint16_t   natural_alignment;

   uint16_t   n_members;        // 0, for scalar, n: for union (members), array(len), bitfield(len), struct( members)
   char       has_object;
   char       has_retainable_object;
};



// returns address of next type in types

static inline char   *
   mulle_objc_signature_supply_next_typeinfo( char *types,
                                              struct mulle_objc_typeinfo *info)
{
   // don't call this yourself, always check types first
   char   *_mulle_objc_signature_supply_next_typeinfo( char *types,
                                                       struct mulle_objc_typeinfo *info);

   if( ! types || ! *types)
      return( NULL);

   return( _mulle_objc_signature_supply_next_typeinfo( types, info));
}


char    *_mulle_objc_signature_next_typeinfo( char *types);
static inline char    *mulle_objc_signature_next_type( char *types)
{
   if( ! types || ! *types)
      return( NULL);

   return( _mulle_objc_signature_next_typeinfo( types));
}

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


#endif /* defined(__MULLE_OBJC__mulle_objc_signature__) */
