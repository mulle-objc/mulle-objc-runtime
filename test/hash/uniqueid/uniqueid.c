//
//  main.m
//  test-md5
//
//  Created by Nat! on 26.02.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_NO_TPS__
# define __MULLE_OBJC_FCS__
# ifdef DEBUG
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif


#include <mulle-objc-runtime/mulle-objc-runtime.h>


int   main( int argc, const char * argv[])
{
   mulle_objc_methodid_t   uniqueid;

   uniqueid = mulle_objc_uniqueid_from_string( "init");
   printf( "%s\n", uniqueid == MULLE_OBJC_INIT_METHODID ? "pass" : "fail");
   uniqueid = mulle_objc_uniqueid_from_string( "mutableCopy");
   printf( "%s\n", uniqueid == MULLE_OBJC_MUTABLECOPY_METHODID ? "pass" : "fail");
   return( 0);
}

