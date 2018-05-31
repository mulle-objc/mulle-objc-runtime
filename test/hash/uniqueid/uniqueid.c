//
//  main.m
//  test-md5
//
//  Created by Nat! on 26.02.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>


int main(int argc, const char * argv[])
{
   mulle_objc_methodid_t   uniqueid;

   uniqueid = mulle_objc_uniqueid_from_string( "init");
   printf( "%s\n", uniqueid == MULLE_OBJC_METHODID( 0x6b1d3731) ? "pass" : "fail");
   return( 0);
}

