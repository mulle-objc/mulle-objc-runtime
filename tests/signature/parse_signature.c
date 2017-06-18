//
//  main.c
//  test-runtime-2
//
//  Created by Nat! on 19/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc_runtime/mulle_objc_runtime.h>

#include <stdio.h>


static char   *signatures[] =
{
   "v40@0:8*16Q24^v32",
   "#16@0:8",
   "@24@0:8@\"NSData\"16",
   "@\"MulleScionObject\"40@0:8@\"<MulleScionOutput>\"16@\"NSMutableDictionary\"24@\"<MulleScionDataSource>\"32",
   0
};


static char   *string_for_paramtype( enum mulle_objc_metaabiparamtype type)
{
   switch( type)
   {
   case mulle_objc_metaabiparamtype_error         : return( "error");
   case mulle_objc_metaabiparamtype_void          : return( "void");
   case mulle_objc_metaabiparamtype_void_pointer  : return( "void *");
   case mulle_objc_metaabiparamtype_param         : return( "_param");
   }
   return( "FAIL");
}


static void test( char *s)
{
   unsigned int  i, n;
   char          *next;

   n =  mulle_objc_signature_count_typeinfos( s);
   printf( "# %s\n", s);
   printf( "number of types = %u\n", n);
   printf( "metaabi param type = %s\n", string_for_paramtype( mulle_objc_signature_get_metaabiparamtype( s)));
   printf( "metaabi return type = %s\n", string_for_paramtype( mulle_objc_signature_get_metaabireturntype( s)));

   for( i = 0; i < n; i++)
   {
      next = mulle_objc_signature_next_type( s);
      printf( "#%d = %.*s\n", i, next ? next - s : 255, s);
      s    = next;
   }
}


int   main( int argc, const char * argv[])
{
   char  **p;

   for( p = signatures; *p; p++)
      test( *p);

   return 0;
}


