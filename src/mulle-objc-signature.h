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

#include "include.h"  // for alignment mulle_objc_vararg.hcode

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mulle-metaabi.h"
#include "mulle-objc-uniqueid.h"

//   union mulle_objc_scalarvalue
//   {
//      char                            *c_atom;
//      intptr_t                        c_bool;
//      char                            *c_charptr;
//      char                            c_chr;
//      struct _mulle_objc_infraclass   c_class;
//      double                          c_dbl;
//      float                           c_flt;
//      id                              c_id;
//      int                             c_int;
//      long                            c_lng;
//      long double                     c_lng_dbl;
//      long long                       c_lng_lng;
//      mulle_objc_methodid_t           c_sel;
//      short                           c_sht;
//      unsigned char                   c_uchr;
//      unsigned int                    c_uint;
//      unsigned long                   c_ulng;
//      unsigned long long              c_ulng_lng;
//      unsigned short                  c_usht;
//      void                            *c_ptr;
//   } v;

// Type modifiers are no longer encoded in mulle-objc. They slow things down
// and there is no interest in them. The DO stuff and GC is also obsolete
//
#ifdef MULLE_OBJC_DEFINE_OBSOLETE_ENCODINGS
# define _C_BYCOPY       'O'
# define _C_BYREF        'R'
# define _C_CONST        'r'  /*  mulle-c compiler doesn't emit it anymore */
# define _C_GCINVISIBLE  '|'  /* lol */
# define _C_ONEWAY       'V'

//
// Candidates for resurrection ? They could be useful for introspecting
// pointer parameters
//
# define _C_IN           'n'
# define _C_INOUT        'N'
# define _C_OUT          'o'
#endif


struct mulle_objc_typeinfo
{
   char          *type;                // not a copy(!) keep your passed in in "types" around, will be past "const"
   char          *pure_type_end;       // if you have "{?=QQ}16", will point just after '}'
   char          *member_type_start;   // if you have "{?=QQ}16" or "[8Q]", will point at first Q
   char          *name;                // @"NSString", @"<X>"  will be "NSString" in quotes!

   unsigned int  n_members;            // 0, for scalar, n: for union (members), array(len), bitfield(len), struct( members)

   size_t        natural_size;
   size_t        bits_size;

   int32_t       invocation_offset;    // only set if used with the enumerator, useful ONLY for NSInvocation!!

   uint16_t      bits_struct_alignment;
   uint16_t      natural_alignment;

   char          has_object;
   char          has_retainable_type;
};


// By changing the supplier function, one can calculate info for a different
// architecture.
typedef int  (*mulle_objc_scalar_typeinfo_supplier_t)( char, struct mulle_objc_typeinfo *);

struct mulle_objc_signaturesupplier
{
   mulle_objc_scalar_typeinfo_supplier_t   supplier;          // leave NULL for default arch
   unsigned int                            index;             // used by the enumerator
   int                                     level;             // init to -1 to convert outer array to pointer
   int32_t                                 invocation_offset; // used by the enumerator
};


typedef void  (*mulle_objc_type_parse_callback_t)( char *type, struct mulle_objc_typeinfo *info, void *userinfo);

MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_signature_supply_scalar_typeinfo( char c, struct mulle_objc_typeinfo *info);

//
// low level routine, you are "supposed to" use a wrapper function like
// _mulle_objc_signature_supply_typeinfo, not _mulle_objc_type_parse. But it
// has a convenient callback...
// Use _mulle_objc_signature_supply_scalar_typeinfo for supplier
MULLE_OBJC_RUNTIME_GLOBAL
char    *_mulle_objc_type_parse( char *type,
                                 int level,
                                 struct mulle_objc_typeinfo *info,
                                 mulle_objc_scalar_typeinfo_supplier_t supplier,
                                 mulle_objc_type_parse_callback_t callback,
                                 void *userinfo);

//
// You usually don't call this yourself, always check types first
// if type is invalid or empty, will return NULL and set errno to EINVAL
// put in NULL for p_offset, unless you are the enumerator and are aware that
// the first typeinfo here will be rval...
//
MULLE_OBJC_RUNTIME_GLOBAL
char   *_mulle_objc_signature_supply_typeinfo( char *types,
                                               struct mulle_objc_signaturesupplier *supplier,
                                               struct mulle_objc_typeinfo *info);

//
// you should be able to iterate through all types of a signature with
// while( mulle_objc_signature_supply_typeinfo( types, NULL, &info))
//
static inline char   *
   mulle_objc_signature_supply_typeinfo( char *types,
                                         struct mulle_objc_signaturesupplier *supplier,
                                         struct mulle_objc_typeinfo *info)
{
   char   *next;

   if( ! types || ! *types)
      return( NULL);

   next = _mulle_objc_signature_supply_typeinfo( types, supplier, info);
   return( next);
}



//
// skip to next type if the types is "@:@@\0"
// the sequence will be ":@@\0" "@@\0" "@\0" "\0" NULL
//
MULLE_OBJC_RUNTIME_GLOBAL
char    *mulle_objc_signature_next_type( char *types);


// kinda the same just a little simpler to interact with
MULLE_OBJC_RUNTIME_GLOBAL
char   *mulle_objc_signature_supply_size_and_alignment( char *type,
                                                        unsigned int *size,
                                                        unsigned int *alignment);

MULLE_OBJC_RUNTIME_GLOBAL
unsigned int    mulle_objc_signature_count_typeinfos( char *types);


//
// -1: error, 0: void, 1: void *, 2: struct
//
// mulle_objc_signature_get_metaabiparamtype will deduce _param correctly from
// the return type also. It needs the complete signature.
//
MULLE_OBJC_RUNTIME_GLOBAL
enum mulle_metaabi_param   mulle_objc_signature_get_metaabiparamtype( char *types);

// this method does not inspect the complete signature! only the return type
MULLE_OBJC_RUNTIME_GLOBAL
enum mulle_metaabi_param   mulle_objc_signature_get_metaabireturntype( char *type);


MULLE_OBJC_RUNTIME_GLOBAL
size_t    _mulle_objc_signature_sizeof_metabistruct( char *type);


//
// not sure of this is really needed or paranoia
// skip prefix fluff of signature
//
MULLE_OBJC_RUNTIME_GLOBAL
char   *_mulle_objc_signature_skip_extendedtypeinfo( char *s);

// this should be sufficiently fast, because the runtime uses this per
// default to check each signature at runtime
static inline int  _mulle_objc_methodsignature_compare( char *a, char *b)
{
   a = strstr( a, "@0:");
   b = strstr( b, "@0:");
   if( ! a)
      return( ! b ? 0 : -1);
   if( ! b)
      return( 1);
   return( strcmp( a, b));
}

MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_typeinfo_compare( struct mulle_objc_typeinfo *a,
                                    struct mulle_objc_typeinfo *b);

MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_methodsignature_compare_lenient( char *a, char *b);

// there is no lenient variant for this
MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_ivarsignature_compare( char *a, char *b);

//
// this also checks the return values, which is all in all not very useful
// in real life.
//
static inline int  _mulle_objc_signature_compare_strict( char *a, char *b)
{
   return( strcmp( a, b));
}


// check if type is '@' '~' '=' '#' (or as member array, union, struct member)
MULLE_OBJC_RUNTIME_GLOBAL
int   mulle_objc_signature_contains_object( char *type);

// check if type is '@' '~' (or as member in array, union, struct member)
MULLE_OBJC_RUNTIME_GLOBAL
int   mulle_objc_signature_contains_retainable_type( char *type);


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


// check if type is '@' '~' (or as member in array, union, struct member)
static inline int   _mulle_objc_type_is_pointer( char *type)
{
   type = _mulle_objc_signature_skip_type_qualifier( type);
   switch( *type)
   {
   case _C_ASSIGN_ID :
   case _C_ATOM      :
   case _C_CHARPTR   :
   case _C_CLASS     :
   case _C_COPY_ID   :
   case _C_FUNCTION  :  // same as undef. undef meaning "undefined size"
   case _C_PTR       :
   case _C_RETAIN_ID :
      return( 1);
   }
   return( 0);
}


static inline int   _mulle_objc_type_is_object( char *type)
{
   type = _mulle_objc_signature_skip_type_qualifier( type);
   switch( *type)
   {
//   case _C_ATOM    : // what's this again ?
   case _C_ASSIGN_ID :
   case _C_CLASS     :
   case _C_COPY_ID   :
   case _C_RETAIN_ID :
      return( 1);
   }
   return( 0);
}


static inline int   _mulle_objc_type_is_fp( char *type)
{
   type = _mulle_objc_signature_skip_type_qualifier( type);
   switch( *type)
   {
   case _C_FLT     :
   case _C_DBL     :
   case _C_LNG_DBL :
   case _C_VECTOR  :  // guess
      return( 1);
   }
   return( 0);
}

//
// Basically a strcmp, that knows when to quit (i.e. the next type is
// coming up. Should be faster than parse and compare. Algorithm believes its
// being fed only well formed types though.
//
int   _mulle_objc_type_is_equal_to_type( char *type_a, char *type_b);


static inline enum mulle_metaabi_param
   _mulle_objc_signature_get_metaabiparamtype( char *type)
{
   type = _mulle_objc_signature_skip_type_qualifier( type);
   return( _mulle_metaabi_get_metaabiparamtype( type));
}


/*
 * PRE-ALPHA
 *
 * _mulle_objc_typeinfo_is_compatible
 * _mulle_objc_ivarsignature_is_compatible
 *
 * These are some crude functions that I need in CoreAnimation and MulleObject
 * but they are too simplistic for general use. Still they are here so I
 * don't have to maintain multiple copies...
 */
int   _mulle_objc_typeinfo_is_compatible( struct mulle_objc_typeinfo *a,
                                          struct mulle_objc_typeinfo *b);
int   _mulle_objc_ivarsignature_is_compatible( char *a, char *b);



static inline void  *
   _mulle_objc_typeinfo_demote_value_to_natural( struct mulle_objc_typeinfo *p,
                                                 void *dst,
                                                 void *src)
{
   switch( *p->type)
   {
   case _C_FLT  :
      *mulle_c_pointer_postincrement( dst, float) = *(double *) src;
      break;
   case _C_SEL  :
      *mulle_c_pointer_postincrement( dst, unsigned int) = *(mulle_objc_methodid_t *) src;
      break;
   case _C_CHR  :
      *mulle_c_pointer_postincrement( dst, int) = *(char *) src;
      break;
   case _C_UCHR :
      *mulle_c_pointer_postincrement( dst, unsigned int) = *(unsigned char *) src;
      break;
   case _C_SHT  :
      *mulle_c_pointer_postincrement( dst, int) = *(short *) src;
      break;
   case _C_USHT :
      *mulle_c_pointer_postincrement( dst, unsigned int) = *(unsigned short *) src;
      break;
   default      :
      memcpy( dst, src, p->natural_size);
      dst = &((char *) dst)[ p->natural_size];
      break;
   }
   return( dst);
}


static inline void *
   _mulle_objc_typeinfo_promote_value_from_natural( struct mulle_objc_typeinfo *p,
                                                    void *dst,
                                                    void *src)
{
   switch( *p->type)
   {
   case _C_FLT  :
      *mulle_c_pointer_postincrement( dst, double) = *(float *) src;
      break;
   case _C_SEL  :
      *mulle_c_pointer_postincrement( dst, mulle_objc_methodid_t) = *(unsigned int *) src;
      break;
   case _C_CHR  :
      *mulle_c_pointer_postincrement( dst, char) = *(int *) src;
      break;
   case _C_UCHR :
      *mulle_c_pointer_postincrement( dst, unsigned char) = *(unsigned int *) src;
      break;
   case _C_SHT  :
      *mulle_c_pointer_postincrement( dst, short) = *(int *) src;
      break;
   case _C_USHT :
      *mulle_c_pointer_postincrement( dst, unsigned short) = *(unsigned int *) src;
      break;
   default      :
      memcpy( dst, src, p->natural_size);
      dst = &((char *) dst)[ p->natural_size];
      break;
   }
   return( dst);
}


//
// The signature enumerator enumerates self, _cmd, args...
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
   char                                  *types;
   struct mulle_objc_signaturesupplier   supplier;
   struct mulle_objc_typeinfo            rval;
};


static inline struct mulle_objc_signatureenumerator
   mulle_objc_signature_enumerate( char *types)
{
   struct mulle_objc_signatureenumerator  rover = { 0 };

   rover.types = (types && *types) ? types : NULL;
   if( ! rover.types)
      return( rover);

   // parse first the incomplete rval
   rover.types =  _mulle_objc_signature_supply_typeinfo( rover.types,
                                                         NULL,
                                                         &rover.rval);
   rover.types = (rover.types && *rover.types) ? rover.types : NULL;

   return( rover);
}


MULLE_C_NONNULL_FIRST
static inline int
   _mulle_objc_signatureenumerator_next( struct mulle_objc_signatureenumerator *rover,
                                         struct mulle_objc_typeinfo *info)
{
   if( ! rover->types)
      return( 0);

   rover->types = _mulle_objc_signature_supply_typeinfo( rover->types,
                                                         &rover->supplier,
                                                         info);
   rover->types = (rover->types && *rover->types) ? rover->types : NULL;
   rover->supplier.index++;
   return( 1);
}


MULLE_C_NONNULL_FIRST
static inline char *
   _mulle_objc_signatureenumerator_get_type( struct mulle_objc_signatureenumerator *rover)
{
   return( rover->rval.type);
}


//
// the invocation_offset will only be valid, if a valid info has been
// passed in for all iterations of the enumerator!
//
MULLE_C_NONNULL_FIRST_SECOND
static inline void
   _mulle_objc_signatureenumerator_rval( struct mulle_objc_signatureenumerator *rover,
                                         struct mulle_objc_typeinfo *info)
{
   assert( rover->supplier.invocation_offset != (int32_t) -1);

   rover->rval.invocation_offset = rover->supplier.invocation_offset;
   memcpy( info, &rover->rval, sizeof( *info));
}


static inline void
   mulle_objc_signatureenumerator_done( struct mulle_objc_signatureenumerator *rover)
{
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
