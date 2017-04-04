//
//  main.m
//  test-md5
//
//  Created by Nat! on 26.02.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc/mulle_objc.h>


// note that: echo "init" | md5  gives a8ba672d93697971031015181d7008c3
// but md5 -s "init" gives e37f0136aa3ffaf149b351f6a4c948e9 which is correct
// I should use echo -n "init"

int main(int argc, const char * argv[])
{
   mulle_objc_methodid_t   uniqueid;

      // e37f0136aa3ffaf149b351f6a4c948e9
   uniqueid = mulle_objc_uniqueid_from_string( "init");
   printf( "%s\n", uniqueid == MULLE_OBJC_METHODID( 0x50c63a23) ? "pass" : "fail");
   return( 0);
}

