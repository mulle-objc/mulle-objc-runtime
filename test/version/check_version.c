//
//  main.c
//  test-runtime-2
//
//  Created by Nat! on 19/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


int   main( int argc, const char * argv[])
{
   if( MULLE_OBJC_RUNTIME_VERSION  != ((MULLE_OBJC_RUNTIME_VERSION_MAJOR << 20) | \
		                       (MULLE_OBJC_RUNTIME_VERSION_MINOR << 8) | \
				       MULLE_OBJC_RUNTIME_VERSION_PATCH))
      return( 1);
   return 0;
}


