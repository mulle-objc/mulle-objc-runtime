//
//  main.c
//  test-runtime-2
//
//  Created by Nat! on 19/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//

#define __TEST_VERSION

#include <mulle-objc-runtime/mulle-objc-jit.inc>

static int JIT_LOAD_VERSION = MULLE_OBJC_RUNTIME_LOAD_VERSION;
static int JIT_MAJOR        = MULLE_OBJC_RUNTIME_VERSION_MAJOR;
static int JIT_MINOR        = MULLE_OBJC_RUNTIME_VERSION_MINOR;
static int JIT_PATCH        = MULLE_OBJC_RUNTIME_VERSION_PATCH;

#undef MULLE_OBJC_RUNTIME_LOAD_VERSION
#undef MULLE_OBJC_RUNTIME_VERSION_MAJOR
#undef MULLE_OBJC_RUNTIME_VERSION_MINOR
#undef MULLE_OBJC_RUNTIME_VERSION_PATCH

#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


int   main( int argc, const char * argv[])
{
   if( MULLE_OBJC_RUNTIME_VERSION != ((MULLE_OBJC_RUNTIME_VERSION_MAJOR << 20) | \
                                      (MULLE_OBJC_RUNTIME_VERSION_MINOR << 8) | \
                                      MULLE_OBJC_RUNTIME_VERSION_PATCH))
   {
      fprintf( stderr, "Runtime header runtime version/compiler version mismatch");
      return( 1);
   }

   if( MULLE_OBJC_RUNTIME_VERSION != ((JIT_MAJOR << 20) | \
      		                          (JIT_MINOR << 8) | \
				                          JIT_PATCH))
   {
      fprintf( stderr, "JIT/Runtime header runtime version mismatch");
      return( 1);
   }

   if( MULLE_OBJC_RUNTIME_LOAD_VERSION != JIT_LOAD_VERSION)
   {
      fprintf( stderr, "JIT/Runtime header load version mismatch");
      return( 1);
   }
   return 0;
}


