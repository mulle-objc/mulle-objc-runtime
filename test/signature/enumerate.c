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
#endif



#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-objc-runtime/mulle-objc-typeinfodump.h>

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

