//
//  mulle_objc_signature.c
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
#include "mulle-objc-signature.h"

#include "mulle-objc-uniqueid.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


static char    *_mulle_objc_type_parse( char *type,
                                        int level,
                                        struct mulle_objc_typeinfo *info,
                                        mulle_objc_scalar_typeinfo_supplier_t supplier);



#define STRUCT_MEMBER_ALIGNMENT( type)  offsetof( struct { char x; type v; }, v)


# pragma mark - type decoding

static inline void   _CLEAR_RUNTIME_TYPE_INFO( struct mulle_objc_typeinfo *info)
{
   info->natural_size          = 0;
   info->natural_alignment     = 0;
   info->bits_struct_alignment = 0;
   info->bits_size             = 0;
   info->has_object            = 0;
   info->has_retainable_object = 0;
}


#define __SUPPLY_RUNTIME_TYPE_INFO( info, type)                        \
   (info)->natural_size          = sizeof( type);                      \
   (info)->natural_alignment     = alignof( type);                     \
   (info)->bits_struct_alignment = STRUCT_MEMBER_ALIGNMENT( type) * 8; \
   (info)->bits_size             = sizeof( type) * 8

#define _SUPPLY_RUNTIME_C_TYPE_INFO( info, type)                       \
   do                                                                  \
   {                                                                   \
      __SUPPLY_RUNTIME_TYPE_INFO( info, type);                         \
      (info)->has_object            = 0;                               \
      (info)->has_retainable_object = 0;                               \
   }                                                                   \
   while( 0)

#define _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, type, retain)            \
   do                                                                  \
   {                                                                   \
      __SUPPLY_RUNTIME_TYPE_INFO( info, type);                         \
      (info)->has_object            = 1;                               \
      (info)->has_retainable_object = retain;                          \
   }                                                                   \
   while( 0)


static int   _mulle_objc_signature_supply_scalar_typeinfo( char c, struct mulle_objc_typeinfo *info)
{
   switch( c)
   {
   case 0            : _CLEAR_RUNTIME_TYPE_INFO( info); return( -1);
   case _C_VOID      : _CLEAR_RUNTIME_TYPE_INFO( info); return( 0);
   case _C_COPY_ID   : _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, struct mulle_objc_object *, 1); return( 0);
   case _C_RETAIN_ID : _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, struct mulle_objc_object *, 1); return( 1);  // because of "@?"
   case _C_ASSIGN_ID : _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, struct mulle_objc_object *, 0); return( 0);
   case _C_CLASS     : _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, struct mulle_objc_class *, 0); return( 0);
   case _C_SEL       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, mulle_objc_methodid_t); return( 0);
   case _C_CHR       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, char); return( 0);
   case _C_BOOL      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, int); return( 0); // it's an enum later
   case _C_UCHR      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, unsigned char); return( 0);
   case _C_SHT       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, short); return( 0);
   case _C_USHT      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, unsigned short); return( 0);
   case _C_INT       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, int); return( 0);
   case _C_UINT      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, unsigned int); return( 0);
   case _C_LNG       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, long); return( 0);
   case _C_ULNG      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, unsigned long); return( 0);
   case _C_LNG_LNG   : _SUPPLY_RUNTIME_C_TYPE_INFO( info, long long); return( 0);
   case _C_ULNG_LNG  : _SUPPLY_RUNTIME_C_TYPE_INFO( info, unsigned long long); return( 0);
   case _C_FLT       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, float); return( 0);
   case _C_DBL       : _SUPPLY_RUNTIME_C_TYPE_INFO( info, double); return( 0);
   case _C_LNG_DBL   : _SUPPLY_RUNTIME_C_TYPE_INFO( info, long double); return( 0);
   case _C_CHARPTR   : _SUPPLY_RUNTIME_C_TYPE_INFO( info, char *); return( 0);
   case _C_UNDEF     : _SUPPLY_RUNTIME_C_TYPE_INFO( info, void *); return( 0);
#ifdef _C_ATOM
   case _C_ATOM      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, char *); return( 0);
#endif
   }

   _CLEAR_RUNTIME_TYPE_INFO( info);
   return( 1);
}


static char  *_mulle_objc_signature_supply_bitfield_typeinfo( char *type,
                                                              int level,
                                                              struct mulle_objc_typeinfo *info)
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
      info->has_object            = 0;
      info->has_retainable_object = 0;
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


static char  *
   _mulle_objc_signature_supply_array_typeinfo( char *type,
                                                int level,
                                                struct mulle_objc_typeinfo *info,
                                                mulle_objc_scalar_typeinfo_supplier_t supplier)
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

   // reuse info, we remember the important parts
   type = _mulle_objc_type_parse( type, level, info, supplier);
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
   info->has_object            = tmp->has_object;
   info->has_retainable_object = tmp->has_retainable_object;
}


static inline void
_update_struct_typeinfo_with_subsequent_member_typeinfo( struct mulle_objc_typeinfo *info,
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

   info->has_object            |= tmp->has_object;
   info->has_retainable_object |= tmp->has_retainable_object;
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


static char  *_mulle_objc_signature_supply_struct_typeinfo( char *type,
                                                            int level,
                                                            struct mulle_objc_typeinfo *info,
                                                            int  (*supply_scalar_typeinfo)( char, struct mulle_objc_typeinfo *))
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
      {
         errno = EINVAL;
         return( NULL);
      }
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

      type = _mulle_objc_type_parse( type, level, tmp, supply_scalar_typeinfo);
      if( ! type)
      {
         errno = EINVAL;
         return( NULL);
      }

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
      info->bits_struct_alignment = tmp->bits_struct_alignment;
   if( tmp->bits_size > info->bits_size)
      info->bits_size = tmp->bits_size;
   if( tmp->natural_size > info->natural_size)
      info->natural_size = tmp->natural_size;

   info->has_object            |= tmp->has_object;
   info->has_retainable_object |= tmp->has_retainable_object;
}


static inline void   _finalize_union_typeinfo( struct mulle_objc_typeinfo *info,
                                               unsigned int n)
{
   if( (uint16_t) n != n)
      abort();

   info->n_members = (uint16_t) n;
}


static char  *
   _mulle_objc_signature_supply_union_typeinfo( char *type,
                                                int level,
                                                struct mulle_objc_typeinfo *info,
                                                mulle_objc_scalar_typeinfo_supplier_t supplier)
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
      {
         errno = EINVAL;
         return( NULL);
      }
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

      type = _mulle_objc_type_parse( type, level, tmp, supplier);
      if( ! type)
         return( NULL);

      // largest of anything wins
      if( tmp)
         _update_union_typeinfo_with_member_typeinfo( info, tmp);
      ++n;
   }
}


//
// this is just for compatibility's sake, there is no actual blocks
// support in mulle-objc
//
typedef void   (*void_function_pointer)( void);

static char  *
   _mulle_objc_signature_supply_block_typeinfo( char *type,
                                                int level,
                                                struct mulle_objc_typeinfo *info)
{
   if( info)
   {
      _SUPPLY_RUNTIME_C_TYPE_INFO( info, void_function_pointer);
   }

   return( type + 1); // skip that extra char
}



static char  *
   _mulle_objc_signature_supply_object_typeinfo( char *type,
                                                 int level,
                                                 struct mulle_objc_typeinfo *info)
{
   if( *type == '?')
      return( _mulle_objc_signature_supply_block_typeinfo( type, level, info));
      // it's a blocks thing

   if( info)
   {
      _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, struct mulle_objc_object *, 1);
   }
   return( type);
}


static inline int   is_multi_character_type( char c)
{
   switch( c)
   {
   case _C_PTR       :
   case _C_ARY_B     :
   case _C_STRUCT_B  :
   case _C_UNION_B   :
   case _C_BFLD      :
   case _C_RETAIN_ID : // thx to "@?", which means what ?
         return( 1);
   }
   return( 0);
}


//
// this should not return NULL, unless the type is incomplete or malformed!
//
// Otherwise returns the end of the partially parsed type (there may be
// extended stuff trailing).e.g. type="[16^]32" will return "32"
//
static char   *
   _mulle_objc_type_parse( char *type,
                           int level,
                           struct mulle_objc_typeinfo *info,
                           int  (*supplier)( char, struct mulle_objc_typeinfo *))
{
   int   isComplex;

   assert( type);

   ++level;
   type = _mulle_objc_signature_skip_type_qualifier( type);

   if( info)
   {
      // _mulle_objc_signature_supply_scalar_typeinfo will set:
      // natural_size
      // natural_alignment
      // bits_struct_alignment
      // bits_size
      // has_object
      // has_retainable_object
      //
      isComplex               = (*supplier)( *type, info);
      // initialize rest:
      info->type              = type;
      info->pure_type_end     = 0;
      info->name              = 0;
      info->invocation_offset = 0;
      info->n_members         = 0;
   }
   else
      isComplex = is_multi_character_type( *type);

   // isComplex == -1: error
   switch( isComplex)
   {
   case 0 :
   // in the non-complex case the parsing of the single character is done
      ++type;  // skip that single char
      return( type);

   case 1 :
      switch( *type++)
      {
      case _C_PTR      :
         if( info)
         {
            if( *type == '?')
            {
               _SUPPLY_RUNTIME_C_TYPE_INFO( info, void_function_pointer);
               return( type + 1);
            }
            _SUPPLY_RUNTIME_C_TYPE_INFO( info, void *);
         }
         return( _mulle_objc_type_parse( type, level, NULL, supplier));  // skip trailing type

      case _C_ARY_B    :
         if( ! level)
         {
            // it's really a pointer
            if( info)
               _SUPPLY_RUNTIME_C_TYPE_INFO( info, void *);
            info = NULL;   // skip array
         }
         return( _mulle_objc_signature_supply_array_typeinfo( type, level, info, supplier));

      case _C_BFLD  :
         return( _mulle_objc_signature_supply_bitfield_typeinfo( type, level, info));

      case _C_RETAIN_ID :
         return( _mulle_objc_signature_supply_object_typeinfo( type, level, info));

      case _C_STRUCT_B :
         return( _mulle_objc_signature_supply_struct_typeinfo( type, level, info, supplier));

      case _C_UNION_B  :
         return( _mulle_objc_signature_supply_union_typeinfo( type, level, info, supplier));
      }
   }

   // default: this shouldn't really happen!
   // passing in "" ??

   errno = EINVAL;
   return( NULL);
}


//
// this returns NULL, if there is no consecutive type
// types must be valid
//
char   *_mulle_objc_signature_supply_typeinfo( char *types,
                                               struct mulle_objc_signaturesupplier *supplier,
                                               struct mulle_objc_typeinfo *info)
{
   char                                  *next;
//   int                                   offset;
//   int                                   sign;
   char                                  c;
   struct mulle_objc_signaturesupplier   dummy;

   if( ! supplier)
   {
      memset( &dummy, 0, sizeof( dummy));
      supplier = &dummy;
   }

   next = _mulle_objc_type_parse( types,
                                  supplier->level,
                                  info,
                                  supplier->supplier
                                    ? supplier->supplier
                                    : _mulle_objc_signature_supply_scalar_typeinfo);
   if( ! next)  // broken ?
      return( NULL);

   if( info)
      info->pure_type_end = next;

   // for blocks only (?) parse some stuff
   if( *next == '<')
      for(;;)
      {
         c = *++next;
         if( ! c)
         {
            errno = EINVAL;
            return( NULL);
         }

         if( c == '>')
         {
            ++next;
            break;
         }
      }

   //
   // parse extended class info
   // @"<NSCopying>" or @"NSString"
   // not considered part of pure_type_end
   //
   if( *next == '"')
   {
      if( info)
         info->name = next;

      while( *++next != '"');
      ++next;  // skip terminator
   }

//   sign = +1;
   if( *next == '-')
      ++next; //, sign = -1;
   if( *next == '+')
      ++next;

//   offset = 0;
   while( *next >= '0' && *next <= '9')
   {
//      offset *= 10;
//      offset += *next - '0';
      ++next;
   }

/*
   if( info)
      info->offset = offset * sign;
*/

   //
   // the offset calculated by the compiler is unfortunately
   // incorrect for the MetaABI block
   //
   if( info)
   {
      //
      // If it was invalidated once, it means you called an iterator with a
      // NULL info sometime in your loop, now offsets are wrong
      //
      assert( supplier->invocation_offset != (unsigned int) -1);

      // for the MetaABI block we want this aligned as safely as possible
      if( supplier->index == 2)
         info->invocation_offset = mulle_address_align( supplier->invocation_offset, alignof( long double));
      else
         info->invocation_offset = mulle_address_align( supplier->invocation_offset, info->bits_struct_alignment / 8);
      supplier->invocation_offset = info->invocation_offset + info->natural_size;
   }
   else
      supplier->invocation_offset = (unsigned int) -1;  // invalidate

   return( next);
}


//
// "types" must be correct
// this does not error checking (we don't have the time)
//
char   *_mulle_objc_signature_skip_extendedtypeinfo( char *s)
{
   char  c;

   //
   // parse extended class info
   // @"<NSCopying>" or @"NSString"
   // not considered part of pure_type_end
   //
   for(;;)
   {
      switch( *s)
      {
         case '<' : // blocksy stuff
            for(;;)
            {
               c = *++s;
               if( ! c)
                  return( NULL);

               if( c == '>')
               {
                  ++s;
                  break;
               }
            }
            break;

         case '"' :
            for(;;)
            {
               c = *++s;
               if( ! c)
                  return( NULL);

               if( c == '"')
               {
                  ++s;
                  break;
               }
            }
            break;

         case '-' :
         case '+' :
            break;

         default :
            if( *s < '0' || *s > '9')
               return( s);
      }
      ++s;
   }
}


char   *mulle_objc_signature_next_type( char *types)
{
   char   *next;

   if( ! types || ! *types)
      return( NULL);

   next = _mulle_objc_type_parse( types, -1, NULL, 0);
   if( ! next)
      return( NULL); // error, errno should be set already

   next = _mulle_objc_signature_skip_extendedtypeinfo( next);
   if( ! next)
      return( NULL);  // error, errno should be set already

   return( next);
}


enum mulle_metaabi_param
   mulle_objc_signature_get_metaabiparamtype( char *types)
{
   int   p_type;
   int   r_type;

   // no parameter is void
   if( ! types)
   {
      errno = EINVAL;
      return( mulle_metaabi_param_error);
   }

   // return value overrides
   r_type = mulle_objc_signature_get_metaabireturntype( types);
   if( r_type == mulle_metaabi_param_struct)
      return( r_type);

   // skip return
   types = mulle_objc_signature_next_type( types);

   // skip self
   types = mulle_objc_signature_next_type( types);

   // skip _cmd
   types = mulle_objc_signature_next_type( types);
   if( ! types)
      return( mulle_metaabi_param_void);

   p_type = _mulle_objc_signature_get_metaabiparamtype( types);
   if( p_type == mulle_metaabi_param_void_pointer)
   {
      // skip current
      types = mulle_objc_signature_next_type( types);
      if( types && *types)
         p_type = mulle_metaabi_param_struct;
   }
   return( p_type);
}


enum mulle_metaabi_param
   mulle_objc_signature_get_metaabireturntype( char *type)
{
   // unspecified return type can't happen though (see above)
   if( ! type)
   {
      errno = EINVAL;
      return( mulle_metaabi_param_error);
   }

   return( _mulle_objc_signature_get_metaabiparamtype( type));
}



char   *mulle_objc_signature_supply_size_and_alignment( char *types,
                                                        unsigned int *size,
                                                        unsigned int *alignment)
{
   struct mulle_objc_typeinfo   info;
   char                         *next;

   if( ! types || ! *types)
      return( NULL);

   // NULL info is cheaper
   if( ! size && ! alignment)
      return( mulle_objc_signature_supply_typeinfo( types, NULL, NULL));

   next = _mulle_objc_signature_supply_typeinfo( types, NULL, &info);
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


int   _mulle_objc_typeinfo_compare( struct mulle_objc_typeinfo *a,
                                    struct mulle_objc_typeinfo *b)
{
   size_t   a_len;
   size_t   b_len;
   int      comparison;

   a_len = a->pure_type_end - a->type;
   b_len = b->pure_type_end - b->type;

   comparison = strncmp( a->type, b->type, a_len < b_len ? a_len : b_len);
   if( comparison)
      return( comparison);

   if( a_len != b_len)
      return( a_len < b_len ? -1 : 1);

   return( 0);
}


int   _mulle_objc_signature_compare_lenient( char *a, char *b)
{
   struct mulle_objc_typeinfo  a_info;
   struct mulle_objc_typeinfo  b_info;
   int                         comparison;

   // skip return value
   a = _mulle_objc_signature_supply_typeinfo( a, NULL, &a_info);
   b = _mulle_objc_signature_supply_typeinfo( b, NULL, &b_info);

   if( ! a)
      return( b ? -1 : 0);
   if( ! b)
      return( 1);

   for(;;)
   {
      a = _mulle_objc_signature_supply_typeinfo( a, NULL, &a_info);
      b = _mulle_objc_signature_supply_typeinfo( b, NULL, &b_info);

      if( ! a)
         return( b ? -1 : 0);
      if( ! b)
         return( 1);

      comparison = _mulle_objc_typeinfo_compare( &a_info, &b_info);
      if( comparison)
         return( comparison);
   }
}


int   mulle_objc_signature_contains_retainableobject( char *type)
{
   struct mulle_objc_typeinfo   info;

   type = mulle_objc_signature_supply_typeinfo( type, NULL, &info);
   if( ! type)
   {
      errno = EINVAL;
      return( -1);
   }
   return( info.has_retainable_object);
}


int   mulle_objc_signature_contains_object( char *type)
{
   struct mulle_objc_typeinfo   info;

   type = mulle_objc_signature_supply_typeinfo( type, NULL, &info);
   if( ! type)
   {
      errno = EINVAL;
      return( -1);
   }
   return( info.has_object);
}


size_t   _mulle_objc_signature_sizeof_metabistruct( char *types)
{
   struct mulle_objc_signatureenumerator   rover;
   struct mulle_objc_typeinfo              info;
   struct mulle_objc_typeinfo              dummy;
   size_t                                  size;

   rover = mulle_objc_signature_enumerate( types);
   {
      _mulle_objc_signatureenumerator_next( &rover, &dummy);
      _mulle_objc_signatureenumerator_next( &rover, &dummy);
      while( _mulle_objc_signatureenumerator_next( &rover, &dummy));
      _mulle_objc_signatureenumerator_rval( &rover, &info);
   }
   mulle_objc_signatureenumerator_done( &rover);

   // compute size of metaABI block
   // (TODO: check that the compiler/runtime has done the right thing here
   // it's OK, if size is a little bit too large.
   size = info.invocation_offset + info.natural_size;
   return( mulle_metaabi_sizeof_union( size));
}

