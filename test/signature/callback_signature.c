//
//  main.c
//  test-runtime-2
//
//  Created by Nat! on 19/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_NO_TPS__
# define __MULLE_OBJC_FCS__
# if defined( DEBUG) || ! defined( __OPTIMIZE__)
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif



#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


static char  *signatures[] =
{
   "{anonymous={?=c*i}{?=c@}[2{?=@*}](?=cq)}",
   "{?=ccfc{?=fc}}",
   "{?=c^{?=f^*}}",
   "*",
   "Q",
   "^v",
   "^s",
   "@\"NSData\"",
   0
};



struct callback_info
{
   FILE                  *fp;
   ptrdiff_t             invocation_offset;
   unsigned int          index;
   struct mulle_buffer   stack;
};


static void   callback( char *type, struct mulle_objc_typeinfo *info, void *userinfo)
{
   struct callback_info  *p = userinfo;
   FILE                  *fp;
   int                   state;

   if( ! info)
   {
      fprintf( fp, "\tNULL\n");
      return;
   }

   switch( *type)
   {
   case _C_STRUCT_B : mulle_buffer_add_byte( &p->stack, *type); break;
   case _C_STRUCT_E : mulle_buffer_pop_byte( &p->stack);        break;
   case _C_UNION_B  : mulle_buffer_add_byte( &p->stack, *type); break;
   case _C_UNION_E  : mulle_buffer_pop_byte( &p->stack);        break;
   case _C_ARY_B    : mulle_buffer_add_byte( &p->stack, *type); break;
   case _C_ARY_E    : mulle_buffer_pop_byte( &p->stack);        break;
   }

   state = mulle_buffer_get_last_byte( &p->stack);

   if( state != _C_STRUCT_E && state != _C_UNION_E && state != _C_ARY_E)
   {
      info->invocation_offset = (int32_t) mulle_address_align( p->invocation_offset,
                                                               p->index == 0
                                                               ? alignof( long double)
                                                               : info->bits_struct_alignment / 8);
      p->invocation_offset = info->invocation_offset + info->natural_size;
      p->index++;
   }


   fprintf( p->fp, "\t(p) type              = %s\n",  type);
   fprintf( p->fp, "\tnatural_size          = %zd\n", info->natural_size);
   fprintf( p->fp, "\tnatural_alignment     = %d\n",  (int) info->natural_alignment);
   fprintf( p->fp, "\tmember_type_start     = %s\n",  info->member_type_start ? info->member_type_start : "NULL");
   fprintf( p->fp, "\thas_object            = %d\n",  info->has_object);
   fprintf( p->fp, "\thas_retainable_type   = %d\n",  info->has_retainable_type);
   fprintf( p->fp, "\tn_members             = %ld\n", (long) info->n_members);
 // fprintf( p->fp, "\tinvocation_offset     = %td\n", (ptrdiff_t) info->invocation_offset);
   fprintf( p->fp, "\tbits_size             = %zd\n", info->bits_size);
   fprintf( p->fp, "\tbits_struct_alignment = %d\n\n",  (int) info->bits_struct_alignment);
}



static void   test( char *signature)
{
   unsigned int                 i, n;
   char                         *next;
   struct mulle_objc_typeinfo   type_info;
   struct callback_info         callback_info;
   char                         *type;

   printf( "type=\"%s\":\n", signature);

   memset( &type_info, 0xDEAD, sizeof( type_info)); // just to mess things up :)

   callback_info = (struct callback_info){ .fp = stdout,
                                           .invocation_offset = 0,
                                           .stack = MULLE_BUFFER_DATA( NULL)
                                          };
   type = _mulle_objc_type_parse( signature,
                                  0,
                                  &type_info,
                                  _mulle_objc_signature_supply_scalar_typeinfo,
                                  callback,
                                  &callback_info);
   mulle_buffer_done( &callback_info.stack);

   if( ! type)
   {
      printf( "SYNTAX ERROR IN SIGNATURE: \"%s\"\n", signature);
      return;
   }
   printf( "\n");
}




int   main( int argc, const char * argv[])
{
   char  **signature;

   for( signature = signatures; *signature; signature++)
   {
      test( *signature);
   }

   return 0;
}


