//
//  mulle_objc_signature.c
//  mulle-objc
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

//

#include "mulle_objc_signature.h"

#include "mulle_objc_uniqueid.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


static char    *__mulle_objc_signature_supply_next_typeinfo( char *type,
                                                            int level,
                                                            struct mulle_objc_typeinfo *info);
char   *_mulle_objc_signature_supply_next_typeinfo( char *types,
                                                    struct mulle_objc_typeinfo *info);


#define STRUCT_MEMBER_ALIGNMENT( type)  offsetof( struct { char x; type v; }, v)


# pragma mark - type decoding

static char   *_mulle_objc_signature_skip_type_qualifier( char *type)
{
   char   c;

   assert( type);

   while( (c = *type) == _C_CONST
#ifdef _C_IN
       || c == _C_IN
#endif
#ifdef _C_INOUT
       || c == _C_INOUT
#endif
#ifdef _C_OUT
       || c == _C_OUT
#endif
#ifdef _C_BYCOPY
       || c == _C_BYCOPY
#endif
#ifdef _C_ONEWAY
       || c == _C_ONEWAY
#endif
      )
   {
      type++;
   }

   return( type);
}


static inline void   _CLEAR_RUNTIME_TYPE_INFO( struct mulle_objc_typeinfo *info)
{
   info->natural_size          = 0;
   info->natural_alignment     = 0;
   info->bits_struct_alignment = 0;
   info->bits_size             = 0;
}


#define _SUPPLY_RUNTIME_TYPE_INFO( info, type, type2)                  \
   info->natural_size          = sizeof( type);                        \
   info->natural_alignment     = alignof( type);                       \
   info->bits_struct_alignment = STRUCT_MEMBER_ALIGNMENT( type) * 8;   \
   info->bits_size             = sizeof( type) * 8


static int   _mulle_objc_signature_supply_scalar_typeinfo( char c, struct mulle_objc_typeinfo *info)
{
   switch( c)
   {
   case 0            : return( 0);
   case _C_VOID      : _CLEAR_RUNTIME_TYPE_INFO( info); return( 0);
   case _C_RETAIN_ID :
   case _C_COPY_ID   :
   case _C_ASSIGN_ID : _SUPPLY_RUNTIME_TYPE_INFO( info, struct mulle_objc_object *, void_pointer); return( 0);
   case _C_CLASS     : _SUPPLY_RUNTIME_TYPE_INFO( info, struct mulle_objc_class *, void_pointer); return( 0);
   case _C_SEL       : _SUPPLY_RUNTIME_TYPE_INFO( info, mulle_objc_methodid_t, void_pointer); return( 0);
   case _C_CHR       : _SUPPLY_RUNTIME_TYPE_INFO( info, char, char); return( 0);
   case _C_UCHR      : _SUPPLY_RUNTIME_TYPE_INFO( info, unsigned char, char); return( 0);
   case _C_SHT       : _SUPPLY_RUNTIME_TYPE_INFO( info, short, short); return( 0);
   case _C_USHT      : _SUPPLY_RUNTIME_TYPE_INFO( info, unsigned short, short); return( 0);
   case _C_INT       : _SUPPLY_RUNTIME_TYPE_INFO( info, int, int); return( 0);
   case _C_UINT      : _SUPPLY_RUNTIME_TYPE_INFO( info, unsigned int, int); return( 0);
   case _C_LNG       : _SUPPLY_RUNTIME_TYPE_INFO( info, long, long); return( 0);
   case _C_ULNG      : _SUPPLY_RUNTIME_TYPE_INFO( info, unsigned long, long); return( 0);
   case _C_LNG_LNG   : _SUPPLY_RUNTIME_TYPE_INFO( info, long long, long_long); return( 0);
   case _C_ULNG_LNG  : _SUPPLY_RUNTIME_TYPE_INFO( info, unsigned long long, long_long); return( 0);
   case _C_FLT       : _SUPPLY_RUNTIME_TYPE_INFO( info, float, float); return( 0);
   case _C_DBL       : _SUPPLY_RUNTIME_TYPE_INFO( info, double, double); return( 0);
   case _C_LNG_DBL   : _SUPPLY_RUNTIME_TYPE_INFO( info, long double, long_double); return( 0);
   case _C_CHARPTR   : _SUPPLY_RUNTIME_TYPE_INFO( info, char *, char_pointer); return( 0);
#ifdef _C_BOOL
   case _C_BOOL      : _SUPPLY_RUNTIME_TYPE_INFO( info, unsigned char, bool); return( 0);
#endif
#ifdef _C_UNDEF
   case _C_UNDEF     : _SUPPLY_RUNTIME_TYPE_INFO( info, void *, void_pointer); return( 0);
#endif
#ifdef _C_ATOM
   case _C_ATOM      : _SUPPLY_RUNTIME_TYPE_INFO( info, char *, char_pointer); return( 0);
#endif
   }
   return( 1);
}


static char  *_mulle_objc_signature_supply_bitfield_info( char *type, int level, struct mulle_objc_typeinfo *info)
{
   int   len;

   len = atoi( type);
   if( (uint16_t) len != len)
      abort();

   while( *type >= '0' && *type <= '9')
      ++type;

   if( info)
   {
      info->n_members             = (uint16_t) len;
      info->bits_size             = info->n_members;
      info->natural_size          = (((info->n_members + 7) / 8) + sizeof( int) - 1) / sizeof( int);
      info->natural_alignment     = 0;    // they have none
      info->bits_struct_alignment = 1;
   }
   return( type);
}


static void   _update_array_typeinfo_with_length( struct mulle_objc_typeinfo *info,
                                                  unsigned int len)
{
   if( (uint16_t) len != len)
      abort();

   info->bits_size    *= len;
   info->natural_size *= len;
   info->n_members     = (uint16_t) len;
}


static char  *_mulle_objc_signature_supply_array_typeinfo( char *type, int level,struct mulle_objc_typeinfo *info)
{
   int   len;
   char  *memoType;

   // compiler lameness
   memoType = NULL;
   len      = 0;
   if( info)
   {
      memoType = info->type;
      len      = atoi( type);
      assert( len);
   }

   while( *type >= '0' && *type <= '9')
      ++type;

   type = __mulle_objc_signature_supply_next_typeinfo( type, level, info);
   if( type)
   {
      if( info)
      {
         _update_array_typeinfo_with_length( info, len);
         info->type = memoType;
      }

      assert( *type == _C_ARY_E);
      ++type;
   }
   return( type);
}


static void   _update_struct_typeinfo_with_first_member_typeinfo( struct mulle_objc_typeinfo *info,
                                                                  struct mulle_objc_typeinfo *tmp)
{
   info->bits_size             = tmp->bits_size;
   info->bits_struct_alignment = tmp->bits_struct_alignment;
   info->natural_alignment     = tmp->natural_alignment;
}


static inline void   _update_struct_typeinfo_with_subsequent_member_typeinfo( struct mulle_objc_typeinfo *info,
                                                                             struct mulle_objc_typeinfo *tmp)
{
   unsigned int   align;

   align = tmp->bits_struct_alignment;
   if( tmp->natural_alignment >= STRUCT_MEMBER_ALIGNMENT( int))
      align = tmp->natural_alignment * 8;

   info->bits_size  = (uint32_t) mulle_address_align( info->bits_size, align);
   info->bits_size += tmp->bits_size;

   if( tmp->natural_alignment > info->natural_alignment)
      info->natural_alignment = tmp->natural_alignment;
   if( tmp->bits_struct_alignment > info->bits_struct_alignment)
      info->bits_struct_alignment = tmp->bits_struct_alignment;

}


static inline void   _finalize_struct_typeinfo( struct mulle_objc_typeinfo *info, unsigned int n)
{
   if( (uint16_t) n != n)
      abort();

   if( info->bits_struct_alignment < alignof( int) * 8)
      info->bits_struct_alignment = (uint16_t) (alignof( int) * 8);  // that's right AFAIK

   info->bits_size    = (uint32_t) mulle_address_align( info->bits_size, info->bits_struct_alignment);
   info->natural_size = info->bits_size / 8;
   info->n_members    = (uint16_t) n;
}


static char  *_mulle_objc_signature_supply_struct_typeinfo( char *type, int level, struct mulle_objc_typeinfo *info)
{
   struct mulle_objc_typeinfo   _tmp;
   struct mulle_objc_typeinfo   *tmp;
   char                         c;
   unsigned int                 n;

   tmp = NULL;
   if( info)
      tmp = &_tmp;

   n = 0;
   for( ;;)
   {
      c = *type++;
      if( ! c)
         return( NULL);
      if( c == _C_STRUCT_E)
      {
         if( info)
            _finalize_struct_typeinfo( info, n);
         return( type);
      }
      if( ! n)
      {
         if( c != '=')
            continue;
      }
      else
         --type;

      type = __mulle_objc_signature_supply_next_typeinfo( type, level, tmp);
      if( ! type)
         return( NULL);

      if( tmp)
      {
         if( ! n)
            _update_struct_typeinfo_with_first_member_typeinfo( info, tmp);
         else
            _update_struct_typeinfo_with_subsequent_member_typeinfo( info, tmp);
      }

      ++n;
   }
}


static void   _update_union_typeinfo_with_member_typeinfo( struct mulle_objc_typeinfo *info,
                                                           struct mulle_objc_typeinfo *tmp)
{
   if( tmp->natural_alignment > info->natural_alignment)
      info->natural_alignment = tmp->natural_alignment;
   if( tmp->bits_struct_alignment > info->bits_struct_alignment)
      info->bits_struct_alignment =  tmp->bits_struct_alignment;
   if( tmp->bits_size > info->bits_size)
      info->bits_size = tmp->bits_size;
   if( tmp->natural_size > info->natural_size)
      info->natural_size = tmp->natural_size;
}


static inline void   _finalize_union_typeinfo( struct mulle_objc_typeinfo *info, unsigned int n)
{
   if( (uint16_t) n != n)
      abort();

   info->n_members = (uint16_t) n;
}


static char  *_mulle_objc_signature_supply_union_typeinfo( char *type, int level, struct mulle_objc_typeinfo *info)
{
   struct mulle_objc_typeinfo   _tmp;
   struct mulle_objc_typeinfo   *tmp;
   char                         c;
   unsigned int                 n;

   tmp = NULL;
   if( info)
      tmp = &_tmp;

   n = 0;
   for(;;)
   {
      c = *type++;
      if( ! c)
         return( NULL);
      if( c == _C_UNION_E)
      {
         if( info)
            _finalize_union_typeinfo( info, n);
         return( type);
      }

      if( ! n)
      {
         if( c != '=')
            continue;
      }
      else
         --type;

      type = __mulle_objc_signature_supply_next_typeinfo( type, level, tmp);
      if( ! type)
         return( NULL);

      // largest of anything wins
      if( tmp)
         _update_union_typeinfo_with_member_typeinfo( info, tmp);
      ++n;
   }
}


static inline int  is_multi_character_type( char c)
{
   switch( c)
   {
   case _C_PTR      :
   case _C_ARY_B    :
   case _C_STRUCT_B :
   case _C_UNION_B  :
   case _C_BFLD     :
         return( 1);
   }
   return( 0);
}


static char    *__mulle_objc_signature_supply_next_typeinfo( char *type,
                                                             int level,
                                                             struct mulle_objc_typeinfo *info)
{
   int   isComplex;
   
   assert( type);
   
   ++level;
   type = _mulle_objc_signature_skip_type_qualifier( type);

   if( info)
   {
      memset( info, 0, sizeof( *info));
      info->type = type;
      isComplex  = _mulle_objc_signature_supply_scalar_typeinfo( *type, info);
   }
   else
      isComplex = is_multi_character_type( *type);

   // this shouldn't really happen!
   // passing in "" ??
   if( ! *type)
      return( NULL);

   // in the non-complex case the parsing of the single character is done
   if( ! isComplex)
   {
      ++type;  // skip that single char

      //
      // parse extended class info
      // @"<NSCopying>" or @"NSString"
      //
      switch( *type)
      {
      case '"' :  // name
         if( info)
            info->name = type;
         
         while( *++type != '"');
         ++type;  // skip terminator
      }
      
      return( type);
   }

   switch( *type++)
   {
   case _C_PTR      :
      if( info)
      {
         _SUPPLY_RUNTIME_TYPE_INFO( info, void *, void_pointer);
      }
      return( __mulle_objc_signature_supply_next_typeinfo( type, level, NULL));  // skip trailing type

   case _C_ARY_B    :
      if( ! level)
      {
         // it's really a pointer
         if( info)
         {
            _SUPPLY_RUNTIME_TYPE_INFO( info, void *, void_pointer);
         }
         info = NULL;   // skip array
      }
      return( _mulle_objc_signature_supply_array_typeinfo( type, level, info));

   case _C_STRUCT_B :
      return( _mulle_objc_signature_supply_struct_typeinfo( type, level, info));

   case _C_UNION_B  :
      return( _mulle_objc_signature_supply_union_typeinfo( type, level, info));

   case _C_BFLD  :
      return( _mulle_objc_signature_supply_bitfield_info( type, level, info));
   }

   return( NULL);
}


char    *_mulle_objc_signature_supply_next_typeinfo( char *types, struct mulle_objc_typeinfo *info)
{
   char   *next;
   int    offset;
   int    sign;

   next = __mulle_objc_signature_supply_next_typeinfo( types, -1, info);
   if( info)
      info->pure_type_end = next;
   if( ! next)
      return( next);

   sign = +1;
   if( *next == '-')
      ++next, sign = -1;
   if( *next == '+')
      ++next;

   offset = 0;
   while( *next >= '0' && *next <= '9')
   {
      offset *= 10;
      offset += *next++ - '0';
   }
   if( info)
      info->offset = offset * sign;

   return( next);
}


//
// "types" must be correct
// this does not error checking (we don't have the time)
//
char   *_mulle_objc_signature_next_typeinfo( char *types)
{
   char   *next;
   
   next = __mulle_objc_signature_supply_next_typeinfo( types, -1, NULL);
   if( ! next)
      return( next);

   switch( *next)
   {
   case '-' :
   case '+' :
      ++next;
   }

   while( *next >= '0' && *next <= '9')
      ++next;

   return( next);
}



#define type_fits_voidptr( x)  ((sizeof( x) <= sizeof( void *)) && (alignof( x) <= alignof( void *)))

enum mulle_objc_metaabiparamtype   mulle_objc_signature_get_metaabiparamtype( char *types)
{
   int   p_type;
   int   r_type;

   if( ! types)
   {
      errno = EINVAL;
      return( mulle_objc_metaabiparamtype_error);
   }

   // return value overrides
   r_type = mulle_objc_signature_get_metaabireturntype( types);
   if( r_type == mulle_objc_metaabiparamtype_param)
      return( r_type);

   types = mulle_objc_signature_next_type( types);

   // self ignore
   types = mulle_objc_signature_next_type( types);

   // _cmd ignore
   types = mulle_objc_signature_next_type( types);

   p_type = mulle_objc_signature_get_metaabireturntype( types);
   if( p_type == mulle_objc_metaabiparamtype_void_pointer)
   {
      // skip current
      types = mulle_objc_signature_next_type( types);

      // get next, if available
      types = mulle_objc_signature_next_type( types);
      if( types)
         p_type = mulle_objc_metaabiparamtype_param;
   }
   return( p_type);
}


enum mulle_objc_metaabiparamtype   mulle_objc_signature_get_metaabireturntype( char *type)
{
   if( ! type)
   {
      errno = EINVAL;
      return( mulle_objc_metaabiparamtype_error);
   }

   type = _mulle_objc_signature_skip_type_qualifier( type);
   switch( *type)
   {
   case 0            :
   case _C_VOID      : return( mulle_objc_metaabiparamtype_void);

#ifdef _C_BOOL
   case _C_BOOL      :
#endif
#ifdef _C_UNDEF
   case _C_UNDEF     :
#endif
#ifdef _C_ATOM
   case _C_ATOM      :
#endif
   case _C_CHARPTR   :
   case _C_PTR       :
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
   case _C_UINT      : return( mulle_objc_metaabiparamtype_void_pointer);
         
   case _C_LNG       : return( type_fits_voidptr( long) ? 1 : 2);
   case _C_ULNG      : return( type_fits_voidptr( unsigned long) ? 1 : 2);
   case _C_LNG_LNG   : return( type_fits_voidptr( long long) ? 1 : 2);
   case _C_ULNG_LNG  : return( type_fits_voidptr( unsigned long long) ? 1 : 2);
   }
   return( mulle_objc_metaabiparamtype_param);
}


char   *mulle_objc_signature_supply_size_and_alignment( char *types, unsigned int *size, unsigned int *alignment)
{
   struct mulle_objc_typeinfo   info;
   char                         *next;

   if( ! types || ! *types)
   {
      memset( &info, 0, sizeof( info));
      return( NULL);
   }

   // NULL info is cheaper
   if( ! size && ! alignment)
      return( mulle_objc_signature_supply_next_typeinfo( types, NULL));

   next = _mulle_objc_signature_supply_next_typeinfo( types, &info);

   if( size)
      *size = info.bits_size >> 3;
   if( alignment)
      *alignment = info.natural_alignment;

   return( next);
}


unsigned int   mulle_objc_signature_count_typeinfos( char *types)
{
   unsigned int   n;

   n = 0;
   while( types = mulle_objc_signature_next_type( types))
      ++n;

   return( n);
}
