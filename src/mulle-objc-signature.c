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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


#define STRUCT_MEMBER_ALIGNMENT( type)  offsetof( struct { char x; type v; }, v)


# pragma mark - type decoding

static inline void   _CLEAR_RUNTIME_TYPE_INFO( struct mulle_objc_typeinfo *info)
{
   info->natural_size          = 0;
   info->natural_alignment     = 0;
   info->bits_struct_alignment = 0;
   info->bits_size             = 0;
   info->has_object            = 0;
   info->has_retainable_type   = 0;
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
      (info)->has_object          = 0;                                 \
      (info)->has_retainable_type = 0;                                 \
   }                                                                   \
   while( 0)

#define _SUPPLY_RUNTIME_CHARPTR_TYPE_INFO( info)                       \
   do                                                                  \
   {                                                                   \
      __SUPPLY_RUNTIME_TYPE_INFO( info, char *);                       \
      (info)->has_object          = 0;                                 \
      (info)->has_retainable_type = 1;                                 \
   }                                                                   \
   while( 0)

#define _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, type, retain)            \
   do                                                                  \
   {                                                                   \
      __SUPPLY_RUNTIME_TYPE_INFO( info, type);                         \
      (info)->has_object          = 1;                                 \
      (info)->has_retainable_type = retain;                            \
   }                                                                   \
   while( 0)


//
// the information here, is intended for use in metaABI construction of
// _param structs in the current architecture. For mulle-vararg and the like
// you should need to take promotion into account and promote before calling
// this. (Or write a different function) For cross-architecture purposes,
// write a different function)
//
int   _mulle_objc_signature_supply_scalar_typeinfo( char c, struct mulle_objc_typeinfo *info)
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
   case _C_CHARPTR   : _SUPPLY_RUNTIME_CHARPTR_TYPE_INFO( info); return( 0);
   case _C_UNDEF     : _SUPPLY_RUNTIME_C_TYPE_INFO( info, void *); return( 0);

   // AI sez: The _C_ATOM encoding is used to represent a special type of
   // pointer called an "atom" in the Objective-C runtime. Atoms are unique,
   // immutable strings used for various internal purposes. (i.e. static,
   // no need to strdup those)
#ifdef _C_ATOM
   case _C_ATOM      : _SUPPLY_RUNTIME_C_TYPE_INFO( info, char *); return( 0);
#endif
   }

   _CLEAR_RUNTIME_TYPE_INFO( info);
   return( 1);
}


static char  *_mulle_objc_signature_supply_bitfield_typeinfo( char *type,
                                                              int level,
                                                              struct mulle_objc_typeinfo *info,
                                                              mulle_objc_type_parse_callback_t callback,
                                                              void *userinfo)
{
   int   len;
   char  *s;

   MULLE_C_UNUSED( level);

   s   = type;
   len = atoi( s);
   if( len < 0)
      abort();

   while( *s >= '0' && *s <= '9')
      ++s;

   if( info)
   {
      info->n_members             = (unsigned int) len;
      info->bits_size             = info->n_members;
      info->natural_size          = (((info->n_members + 7) / 8) + sizeof( int) - 1) / sizeof( int);
      info->natural_alignment     = 0;    // they have none
      info->bits_struct_alignment = 1;
      info->has_object            = 0;
      info->has_retainable_type   = 0;
      (*callback)( &type[ -1], info, userinfo);
   }
   return( s);
}


// TODO: alignment ?
static void   _update_array_typeinfo_with_length( struct mulle_objc_typeinfo *info,
                                                  struct mulle_objc_typeinfo *tmp)
{
   info->bits_size             = info->n_members * tmp->bits_size;
   info->natural_size          = info->n_members * tmp->natural_size;
   info->natural_alignment     = tmp->natural_alignment;    // they have none
   info->bits_struct_alignment = tmp->bits_struct_alignment;
   info->has_object            = tmp->has_object;
   info->has_retainable_type   = tmp->has_retainable_type;
}

// '[22d]'
static char  *
   _mulle_objc_signature_supply_array_typeinfo( char *type,
                                                int level,
                                                struct mulle_objc_typeinfo *info,
                                                mulle_objc_scalar_typeinfo_supplier_t supplier,
                                                mulle_objc_type_parse_callback_t callback,
                                                void *userinfo)
{
   int                          len;
   char                         *memoType;
   struct mulle_objc_typeinfo   _tmp;
   struct mulle_objc_typeinfo   *tmp;
   // compiler lameness

   tmp       = info ? &_tmp : NULL;
   memoType  = type;
   len       = 0;

   while( *type >= '0' && *type <= '9')
   {
      len *= 10;
      len += *type - '0';
      ++type;
   }

   if( info)
   {
      info->n_members         = len;
      info->member_type_start = type;
      if( info->n_members != (unsigned int) len)
         abort();
      (*callback)( &memoType[ -1], info, userinfo);
   }

   type = _mulle_objc_type_parse( type, level, tmp, supplier, callback, userinfo);
   if( type)
   {
      assert( *type == _C_ARY_E);
      ++type;

      if( info)
      {
         _update_array_typeinfo_with_length( info, tmp);
         (*callback)( &type[ -1], info, userinfo);
      }
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
   info->has_retainable_type   = tmp->has_retainable_type;
}


static inline void
   _update_struct_typeinfo_with_subsequent_member_typeinfo( struct mulle_objc_typeinfo *info,
                                                            struct mulle_objc_typeinfo *tmp)
{
   unsigned int   align;

   align = tmp->bits_struct_alignment;
//   // not sure what this was about...
//   if( tmp->natural_alignment >= STRUCT_MEMBER_ALIGNMENT( int))
//      align = tmp->natural_alignment * 8;

   info->bits_size  = (uint32_t) mulle_address_align( info->bits_size, align);
   info->bits_size += tmp->bits_size;

   //
   // the overall alignment of a struct is that of the largest member,
   // presumably so that you can create an array out of it, and each struct
   // is properly aligned for the member
   //
   if( tmp->natural_alignment > info->natural_alignment)
      info->natural_alignment = tmp->natural_alignment;
   if( tmp->bits_struct_alignment > info->bits_struct_alignment)
      info->bits_struct_alignment = tmp->bits_struct_alignment;

   info->has_object          |= tmp->has_object;
   info->has_retainable_type |= tmp->has_retainable_type;
}


static inline void   _finalize_struct_typeinfo( struct mulle_objc_typeinfo *info, unsigned int n)
{
   if( (uint32_t) n != n)
      abort();

//   if( info->bits_struct_alignment < alignof( int) * 8)
//      info->bits_struct_alignment = (uint16_t) (alignof( int) * 8);  // that's right AFAIK

   info->bits_size    = (uint32_t) mulle_address_align( info->bits_size, info->bits_struct_alignment);
   info->natural_size = info->bits_size / 8;
   info->n_members    = n;
}


static char  *
   _mulle_objc_signature_supply_struct_typeinfo( char *type,
                                                 int level,
                                                 struct mulle_objc_typeinfo *info,
                                                 int  (*supply_scalar_typeinfo)( char, struct mulle_objc_typeinfo *),
                                                 mulle_objc_type_parse_callback_t callback,
                                                 void *userinfo)
{
   struct mulle_objc_typeinfo   _tmp;
   struct mulle_objc_typeinfo   *tmp;
   char                         *memoType;
   char                         c;
   unsigned int                 n;

   memoType = type;
   tmp      = info ? &_tmp : NULL;

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
         {
            _finalize_struct_typeinfo( info, n);
            (*callback)( &type[ -1], info, userinfo);
         }
         return( type);
      }
      if( ! n)
      {
         if( c != '=')
            continue;
         if( info)
         {
            info->name              = memoType;
            info->member_type_start = type;
            (*callback)( &memoType[ -1], info, userinfo);
         }
      }
      else
         --type;

      type = _mulle_objc_type_parse( type, level, tmp, supply_scalar_typeinfo, callback, userinfo);
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

   info->has_object          |= tmp->has_object;
   info->has_retainable_type |= tmp->has_retainable_type;
}


static inline void   _finalize_union_typeinfo( struct mulle_objc_typeinfo *info,
                                               unsigned int n)
{
   if( (uint32_t) n != n)
      abort();

   info->n_members = (uint32_t) n;
}


static char  *
   _mulle_objc_signature_supply_union_typeinfo( char *type,
                                                int level,
                                                struct mulle_objc_typeinfo *info,
                                                mulle_objc_scalar_typeinfo_supplier_t supplier,
                                                mulle_objc_type_parse_callback_t callback,
                                                void *userinfo)
{
   struct mulle_objc_typeinfo   _tmp;
   struct mulle_objc_typeinfo   *tmp;
   char                         c;
   unsigned int                 n;
   char                         *memoType;

   memoType = type;
   tmp      = info ? &_tmp : NULL;

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
         {
            _finalize_union_typeinfo( info, n);
            (*callback)( &type[ -1], info, userinfo);
         }
         return( type);
      }

      if( ! n)
      {
         if( c != '=')
            continue;

         // start of members!
         if( info)
         {
            info->name              = memoType;
            info->member_type_start = type;
            (*callback)( &memoType[ -1], info, userinfo);
         }
      }
      else
         --type;

      type = _mulle_objc_type_parse( type, level, tmp, supplier, callback, userinfo);
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
                                                struct mulle_objc_typeinfo *info,
                                                mulle_objc_type_parse_callback_t callback,
                                                void *userinfo)
{
   MULLE_C_UNUSED( level);

   if( info)
   {
      _SUPPLY_RUNTIME_C_TYPE_INFO( info, void_function_pointer);
      (*callback)( &type[ -1], info, userinfo);
   }

   return( type + 1); // skip that extra char
}


static char  *
   _mulle_objc_signature_supply_object_typeinfo( char *type,
                                                 int level,
                                                 struct mulle_objc_typeinfo *info,
                                                 mulle_objc_type_parse_callback_t callback,
                                                 void *userinfo)
{
   if( *type == '?')
      return( _mulle_objc_signature_supply_block_typeinfo( type, level, info, callback, userinfo));
      // it's a blocks thing

   if( info)
   {
      _SUPPLY_RUNTIME_OBJC_TYPE_INFO( info, struct mulle_objc_object *, 1);
      (*callback)( &type[ -1], info, userinfo);
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
// level can be -1 for special cases..
//
char   *
   _mulle_objc_type_parse( char *type,
                           int level,
                           struct mulle_objc_typeinfo *info,
                           int  (*supplier)( char, struct mulle_objc_typeinfo *),
                           mulle_objc_type_parse_callback_t callback,
                           void *userinfo)
{
   int   isComplex;

   assert( type);
   assert( (callback && info) || ! callback);  // need info if you want callbacks

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
      // has_retainable_type
      //
      isComplex               = (*supplier)( *type, info);
      // initialize rest:
      info->type              = type;
      info->member_type_start = 0;
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
      if( info)
         (*callback)( type, info, userinfo);
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
               (*callback)( &type[ -1], info, userinfo);
               return( type + 1);
            }
            _SUPPLY_RUNTIME_C_TYPE_INFO( info, void *);
            (*callback)( &type[ -1], info, userinfo);
         }
         //
         // MEMO: if it becomes necessary to pass callback and userinfo here
         //       check out NSInvocation and set _C_PTR_PARSE_SENDS_CALLBACKS
         //
         return( _mulle_objc_type_parse( type, level, NULL, supplier, 0, NULL));  // skip trailing type

      case _C_ARY_B    :
         if( ! level)  // if level was given as -1
         {
            // it's really a pointer
            if( info)
            {
               _SUPPLY_RUNTIME_C_TYPE_INFO( info, void *);
               (*callback)( &type[ -1], info, userinfo);
            }
            info = NULL;   // skip array
         }
         return( _mulle_objc_signature_supply_array_typeinfo( type, level, info, supplier, callback, userinfo));

      case _C_BFLD  :
         return( _mulle_objc_signature_supply_bitfield_typeinfo( type, level, info, callback, userinfo));

      case _C_RETAIN_ID :
         return( _mulle_objc_signature_supply_object_typeinfo( type, level, info, callback, userinfo));

      case _C_STRUCT_B :
         return( _mulle_objc_signature_supply_struct_typeinfo( type, level, info, supplier, callback, userinfo));

      case _C_UNION_B  :
         return( _mulle_objc_signature_supply_union_typeinfo( type, level, info, supplier, callback, userinfo));
      }
   }

   // default: this shouldn't really happen!
   // passing in "" ??

   errno = EINVAL;
   return( NULL);
}



static void   dummy_callback( char *type, struct mulle_objc_typeinfo *info, void *userinfo)
{
   MULLE_C_UNUSED( type);
   MULLE_C_UNUSED( info);
   MULLE_C_UNUSED( userinfo);
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
                                  : _mulle_objc_signature_supply_scalar_typeinfo,
                                  info ? dummy_callback : 0,
                                  NULL);
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
   // incorrect for the MetaABI block. Here we do something for the
   // NSInvocation...
   //
   if( info)
   {
      //
      // If it was invalidated once, it means you called an iterator with a
      // NULL info sometime in your loop, now offsets are wrong
      //
      assert( supplier->invocation_offset != -1);

      // for the MetaABI block we want this aligned as safely as possible
      info->invocation_offset     = (int32_t) mulle_address_align( supplier->invocation_offset,
                                                                   supplier->index == 2
                                                                   ? alignof( long double)
                                                                   : info->bits_struct_alignment / 8);
      supplier->invocation_offset = (int32_t) (info->invocation_offset + info->natural_size);
   }
   else
      supplier->invocation_offset = -1;  // invalidate

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

   next = _mulle_objc_type_parse( types, -1, NULL, 0, 0, NULL);
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
      *size = (unsigned int) (info.bits_size >> 3);
   if( alignment)
      *alignment = info.natural_alignment;

   return( next);
}



unsigned int   mulle_objc_signature_count_typeinfos( char *types)
{
   unsigned int   n;

   n = 0;
   while( (types = mulle_objc_signature_next_type( types)))
      ++n;

   return( n);
}


int   _mulle_objc_ivarsignature_compare( char *a, char *b)
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

   comparison = _mulle_objc_typeinfo_compare( &a_info, &b_info);
   return( comparison);
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


int   _mulle_objc_methodsignature_compare_lenient( char *a, char *b)
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


int   mulle_objc_signature_contains_retainable_type( char *type)
{
   struct mulle_objc_typeinfo   info;

   type = mulle_objc_signature_supply_typeinfo( type, NULL, &info);
   if( ! type)
   {
      errno = EINVAL;
      return( -1);
   }
   return( info.has_retainable_type);
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


int   _mulle_objc_type_is_equal_to_type( char *a, char *b)
{
   int   c, d;
   int   level;

   a = _mulle_objc_signature_skip_type_qualifier( a);
   b = _mulle_objc_signature_skip_type_qualifier( b);
   c = *a;
   d = *b;
   if( c != d)
      return( 0);

   level = 0;
   switch( c)
   {
   case _C_RETAIN_ID :
   case _C_ASSIGN_ID :
   case _C_COPY_ID   :
      c = *++a;
      d = *++b;
      if( c != '<')
         return( 1);
      do
      {
         if( c != d || ! c)
            return( 0);
         c = *++a;
         d = *++b;
      }
      while( c != '>');
      return( 1);

   case _C_STRUCT_B :
   case _C_UNION_B  :
   case _C_ARY_B    :
      for(;;)
      {
         c = *++a;
         d = *++b;
         if( c != d || ! c)
            return( 0);

         switch( c)
         {
         case _C_STRUCT_B :
         case _C_UNION_B  :
         case _C_ARY_B    :
            level++;
            break;

         case _C_STRUCT_E :
         case _C_UNION_E  :
         case _C_ARY_E    :
            if( ! --level)
               return( 0);
         }
      }
      return( 1);

   default :
      return( 1);
   }
}


/*
 * This code checks if two ivars are binary compatible.
 * f.e. uint32_t and int32_t are deemed compatible
 * struct { float x, y } and float f[ 2] are also compatible and so on
 * char and int are not compatible, and float and int are neither.
 * pointers are considered compatible, but char *s and void * are not,
 * different encodings!
 */ 
struct mulle_signature_array_state
{
   char     *field;
   size_t   repeat;
};

static inline struct mulle_signature_array_state   
   mulle_signature_array_state_make( char *field, size_t repeat)
{
   return( (struct mulle_signature_array_state) 
           { 
              .field  = field, 
              .repeat = repeat
           });
}


//
//  the field enumerator has 256 bytes of storage, the max number of nested
//  256 - (16 + (8 * 16) + 8) // 256 - 160 = 96 
// 
#define MAX_ARRAY_DEPTH      8
#define MAX_AGGREGATE_DEPTH  (256 - sizeof( char *) * 2  \
                                  - sizeof( struct mulle_signature_array_state) * MAX_ARRAY_DEPTH \
                                  - sizeof( int32_t) * 2)

struct mulle_signature_field_enumerator
{
   char      *curr;
   char      *sentinel;

   int32_t   array_depth;
   struct mulle_signature_array_state    array_state[ MAX_ARRAY_DEPTH];

   int32_t   depth;
   char      stack[ MAX_AGGREGATE_DEPTH];
};


static inline struct mulle_signature_field_enumerator   
   mulle_signature_field_enumerator_make( char *s,
                                          size_t len)
{
   MULLE_C_ASSERT( MAX_AGGREGATE_DEPTH >= 32);

   return( (struct mulle_signature_field_enumerator) 
           { 
              .curr     = s, 
              .sentinel = &s[ len]
           });
}

static int  
   _mulle_signature_field_enumerator_skip_past( struct mulle_signature_field_enumerator *p, 
                                                         int c)
{
   for(;;)
   {
      if( p->curr >= p->sentinel)
         return( 0);
      if( *p->curr++ == c)
         return( 1);
   }
}


// must have a number
static int  
   _mulle_signature_field_enumerator_skip_past_number( struct mulle_signature_field_enumerator *p)
{
   int   c;
   int   nr;

   nr = 0;
   for(;;)
   {
      if( p->curr >= p->sentinel)
         return( 0);
      c = *p->curr;
      if( c < '0' || c > '9')
         return( nr);
      nr = (nr * 10) + (c - '0');
      ++p->curr;
   }
}



static int  _mulle_signature_field_enumerator_next( struct mulle_signature_field_enumerator *p)
{
   int      c;
   int      aggregate;
   struct mulle_signature_array_state  *array_state;
   size_t   repeat;

   if( p->curr == p->sentinel)
      return( 0);

   // inside struct or union
   aggregate = 0;
   if( p->depth)
   {
next:
      aggregate = p->stack[ p->depth - 1];
      if( aggregate == _C_ARY_B)
      {
         array_state = &p->array_state[ p->array_depth - 1];
         if( array_state->repeat)
         {
            --array_state->repeat;
            p->curr = array_state->field;
         }
      }
   }

   for(;;)
   {
      c = *p->curr++;
      switch( c)
      {
      case _C_ARY_B : // [5i] : grab number and into repeater
         p->stack[ p->depth] = c;
         if( ++p->depth >= MAX_AGGREGATE_DEPTH)
         {
            assert( p->depth < MAX_AGGREGATE_DEPTH);
            return( 0);
         }
         if( p->array_depth >= MAX_ARRAY_DEPTH)
         {
            assert( p->array_depth < MAX_ARRAY_DEPTH);
            return( 0);
         }

         repeat = _mulle_signature_field_enumerator_skip_past_number( p);
         p->array_state[ p->array_depth++] = mulle_signature_array_state_make( p->curr, repeat);
         goto next;

      case _C_ARY_E  :
         assert( aggregate == _C_ARY_B);
         --p->depth;
         if( p->depth <= 0)
         {
            p->sentinel = p->curr;
            return( 0);
         }

         --p->array_depth;
         assert( p->array_depth >= 0);

         goto next;

      case _C_STRUCT_B :
         p->stack[ p->depth] = c;
         if( ++p->depth >= MAX_AGGREGATE_DEPTH)
         {
            assert( p->depth < MAX_AGGREGATE_DEPTH);
            return( 0);
         }
         // skip past '=' a get first field
         if( ! _mulle_signature_field_enumerator_skip_past( p, '='))
            return( 0);
         goto next;

      case _C_STRUCT_E :
         assert( aggregate == _C_STRUCT_B);
         --p->depth;
         if( p->depth <= 0)
         {
            p->sentinel = p->curr;
            return( 0);
         }
         goto next;      

      case _C_UNION_B  :
         p->stack[ p->depth] = c;
         if( ++p->depth >= MAX_AGGREGATE_DEPTH)
         {
            assert( p->depth < MAX_AGGREGATE_DEPTH);
            return( 0);
         }
         // skip past '=' a get to first field then need to ensure
         if( ! _mulle_signature_field_enumerator_skip_past( p, '='))
            return( 0);
         // we put _C_UNION_B first into the stack and once we read
         // first field it will become _C_UNION_E 
         goto next;      

      case _C_UNION_E  :
         assert( aggregate == _C_UNION_B || aggregate == _C_UNION_E);
         --p->depth;
         if( p->depth <= 0)
         {
            p->sentinel = p->curr;
            return( 0);
         }
         goto next;

      default : 
         // close it down if we have no depth
         // if we are in a union with first field read we skip
         if( aggregate == _C_UNION_E)
            continue;
         // mark this union has having read once
         if( aggregate == _C_UNION_B)
            p->stack[ p->depth - 1] = _C_UNION_E;
         if( p->depth <= 0)
            p->sentinel = p->curr;
         return( c);
      }
   }
}


// Function to compare two types for binary compatibility
static int   _mulle_objc_types_are_binary_compatible( char* a, size_t a_len, 
                                                      char* b, size_t b_len) 
{
   struct mulle_signature_field_enumerator  a_rover;
   struct mulle_signature_field_enumerator  b_rover;
   int    a_encode, b_encode;

   a_rover = mulle_signature_field_enumerator_make( a, a_len);
   b_rover = mulle_signature_field_enumerator_make( b, b_len);

   for(;;)
   {
      a_encode = _mulle_signature_field_enumerator_next( &a_rover);
      b_encode = _mulle_signature_field_enumerator_next( &b_rover);

      if( a_encode == 0)
         return( b_encode == 0);
      if( b_encode == 0)
         return( 0);

      switch( a_encode) 
      {
      case _C_ASSIGN_ID :
      case _C_COPY_ID   :
      case _C_RETAIN_ID :
      case _C_CLASS     :
          if( b_encode == _C_ASSIGN_ID 
              || b_encode == _C_COPY_ID 
              || b_encode == _C_RETAIN_ID 
              || b_encode == _C_CLASS) 
          {
            continue;
          }
          return( 0);

      case _C_LNG_LNG  :
      case _C_ULNG_LNG :
          if( b_encode == _C_LNG_LNG || b_encode == _C_ULNG_LNG)
              continue;
          return( 0);

      case _C_LNG  :
      case _C_ULNG :
          if( b_encode == _C_LNG || b_encode == _C_ULNG)
              continue;
          return( 0);

      case _C_INT  :
      case _C_UINT :
          if( b_encode == _C_INT || b_encode == _C_UINT)
              continue;
          return( 0);

      case _C_SHT  :
      case _C_USHT :
          if( b_encode == _C_SHT || b_encode == _C_USHT)
              continue;
          return( 0);

      case _C_CHR  :
      case _C_UCHR :
          if( b_encode == _C_CHR || b_encode == _C_UCHR)
              continue;
          return( 0);
      }

      if( a_encode != b_encode)
         return( 0);
   }
}


int   _mulle_objc_typeinfo_is_binary_compatible( struct mulle_objc_typeinfo *a, 
                                                 struct mulle_objc_typeinfo *b)
{
    size_t   a_len;    
    size_t   b_len;

    a_len = (size_t) (a->pure_type_end - a->type);
    b_len = (size_t) (b->pure_type_end - b->type);

    return( _mulle_objc_types_are_binary_compatible( a->type, a_len, 
                                                    b->type, b_len));
}


int   _mulle_objc_ivarsignature_is_binary_compatible( char *a, char *b)
{
   struct mulle_objc_typeinfo   a_info;
   struct mulle_objc_typeinfo   b_info;

   // skip return value
   a = _mulle_objc_signature_supply_typeinfo( a, NULL, &a_info);
   b = _mulle_objc_signature_supply_typeinfo( b, NULL, &b_info);

   if( ! a)
      return( b ? 0 : 1);
   if( ! b)
      return( 0);

   return( _mulle_objc_typeinfo_is_binary_compatible( &a_info, &b_info));
}

