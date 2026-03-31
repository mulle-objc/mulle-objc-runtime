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


static char   *signatures[] =
{
   "v40@0:8*16Q24^v32",
   "#16@0:8",
   "@24@0:8@\"NSData\"16",
   "@\"MulleScionObject\"40@0:8@\"<MulleScionOutput>\"16@\"NSMutableDictionary\"24@\"<MulleScionDataSource>\"32",
   "@28@0:8i16@?<v@?>20",
   "i28@0:8q16f24",
   0
};


static void   mulle_objc_typeinfo_dump_to_file( struct mulle_objc_typeinfo *info,
                                                char *indent,
                                                FILE *fp)
{
   mulle_fprintf( fp, "%stype=%.*s\n", indent, (int) (info->pure_type_end - info->type), info->type);
   mulle_fprintf( fp, "%sinvocation_offset=%d\n", indent, (int) info->invocation_offset);
   mulle_fprintf( fp, "%snatural_size=%u\n", indent, (unsigned int) info->natural_size);
   mulle_fprintf( fp, "%sbits_size=%u\n", indent, (unsigned int) info->bits_size);
   mulle_fprintf( fp, "%sbits_struct_alignment=%u\n", indent, (unsigned int) info->bits_struct_alignment);
   mulle_fprintf( fp, "%snatural_alignment=%u\n", indent, (unsigned int) info->natural_alignment);
   mulle_fprintf( fp, "%sn_members= %d\n", indent, info->n_members);
   mulle_fprintf( fp, "%shas_object= %d\n", indent, info->has_object);
}



static int   test_info( char *s)
{
   unsigned int                            i;
   struct mulle_objc_signatureenumerator   rover;
   struct mulle_objc_typeinfo              info;

   rover = mulle_objc_signature_enumerate( s);

   i = 0;
   while( _mulle_objc_signatureenumerator_next( &rover, &info))
   {
      printf( "#%d\n", i);
      mulle_objc_typeinfo_dump_to_file( &info, "\t", stdout);
      ++i;
   }

   _mulle_objc_signatureenumerator_rval( &rover, &info);
   printf( "rval\n");
   mulle_objc_typeinfo_dump_to_file( &info, "\t", stdout);

   mulle_objc_signatureenumerator_done( &rover);
   return( 0);
}




int   main( int argc, const char * argv[])
{
   char  **p;

   for( p = signatures; *p; p++)
   {
      printf( "Signature: %s\n", *p);
      if( test_info( *p))
         return( 1);
      putchar( '\n');
   }

   return 0;
}

