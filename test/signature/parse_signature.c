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


static char   *string_for_paramtype( enum mulle_metaabi_param type)
{
   switch( type)
   {
   case mulle_metaabi_param_error         : return( "error");
   case mulle_metaabi_param_void          : return( "void");
   case mulle_metaabi_param_void_pointer  : return( "void *");
   case mulle_metaabi_param_struct        : return( "struct {...} *");
   }
   return( "FAIL");
}


static int   test( char *s)
{
   unsigned int  i, n;
   char          *next;

   n = mulle_objc_signature_count_typeinfos( s);
   printf( "signature=\"%s\"\n", s);
   printf( "number of types = %u\n", n);
   printf( "metaabi param type = %s\n", string_for_paramtype( mulle_objc_signature_get_metaabiparamtype( s)));
   printf( "metaabi return type = %s\n", string_for_paramtype( mulle_objc_signature_get_metaabireturntype( s)));

   for( i = 0; i < n; i++)
   {
      next = mulle_objc_signature_next_type( s);
      printf( "#%d = %.*s\n", i, next ? (int) (next - s) : 255, s);
      s    = next;
   }

   if( ! s)
   {
      fprintf( stderr, "unexpected NULL in %s\n", __PRETTY_FUNCTION__);
      return( 1);
   }

   if( *s)
   {
      fprintf( stderr, "unexpected tail \"%s\" in %s\n", s, __PRETTY_FUNCTION__);
      return( 1);
   }
   return( 0);
}


static int   test_info( char *s)
{
   unsigned int  i, n;
   char          *next;
   struct mulle_objc_typeinfo   info;

   n = mulle_objc_signature_count_typeinfos( s);

   printf( "signature=\"%s\"\n", s);
   printf( "number of types = %u\n", n);

   for( i = 0; i < n; i++)
   {
      next = mulle_objc_signature_supply_typeinfo( s, NULL, &info);

      printf( "#%d type= %s\n", i, info.type ? info.type : "NULL");
      printf( "#%d pure_type_end= %s\n", i, info.pure_type_end ? info.pure_type_end : "NULL");
      printf( "#%d name= %s\n", i, info.name ? info.name : "NULL");
      printf( "#%d n_members= %ld\n", i, (long) info.n_members);
      printf( "#%d has_object= %d\n", i, info.has_object);
      printf( "#%d has_object= %d\n", i, info.has_object);

      s = next;
   }

   if( ! s)
   {
      fprintf( stderr, "unexpected NULL in %s\n", __PRETTY_FUNCTION__);
      return( 1);
   }
   if( *s)
   {
      fprintf( stderr, "unexpected tail \"%s\" in %s\n", s, __PRETTY_FUNCTION__);
      return( 1);
   }
   return( 0);
}



static int   test_while( char *s)
{
   unsigned int   i, n;
   char           *next;

   n = mulle_objc_signature_count_typeinfos( s);
   i = 0;
   while( (next = mulle_objc_signature_next_type( s)))
   {
      printf( "#%d = %.*s\n", i, next ? (int) (next - s) : 255, s);
      s = next;
      ++i;
   }

   if( i != n)
   {
      fprintf( stderr, "unexpected loop mismatch in %s\n", __PRETTY_FUNCTION__);
      return( 1);
   }

   return( 0);
}


static int   test_while_info( char *s)
{
   struct mulle_objc_typeinfo   info;
   unsigned int                 i, n;
   char                        *next;

   n = mulle_objc_signature_count_typeinfos( s);
   i = 0;

   while( (next = mulle_objc_signature_supply_typeinfo( s, NULL, &info)))
   {
      printf( "#%d: type=%s\n", i, info.type ? info.type : "NULL");
      s = next;
      ++i;
   }

   if( i != n)
   {
      fprintf( stderr, "unexpected loop mismatch in %s\n", __PRETTY_FUNCTION__);
      return( 1);
   }

   return( 0);
}


int   main( int argc, const char * argv[])
{
   char  **p;

   for( p = signatures; *p; p++)
   {
      if( test( *p))
         return( 1);
      putchar( '\n');

      if( test_while( *p))
         return( 1);
      putchar( '\n');

      if( test_info( *p))
         return( 1);
      putchar( '\n');

      if( test_while_info( *p))
         return( 1);
      putchar( '\n');
   }

   return 0;
}


