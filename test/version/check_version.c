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

#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_NO_TPS__
# define __MULLE_OBJC_FCS__
#endif


#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


int   main( int argc, const char * argv[])
{
   uint32_t   version;

   version = ((MULLE_OBJC_RUNTIME_VERSION_MAJOR << 20) | \
              (MULLE_OBJC_RUNTIME_VERSION_MINOR << 8) | \
              MULLE_OBJC_RUNTIME_VERSION_PATCH);   
   if( MULLE_OBJC_RUNTIME_VERSION != version)
   {
      fprintf( stderr, "Runtime header runtime version/compiler version mismatch (%u %lu)",
         version, (unsigned long) MULLE_OBJC_RUNTIME_VERSION);
      return( 1);
   }

   version = ((JIT_MAJOR << 20) | (JIT_MINOR << 8) |JIT_PATCH);
   if( MULLE_OBJC_RUNTIME_VERSION != version)
   {
      fprintf( stderr, "JIT/Runtime header runtime version mismatch (%u %lu)\n",
         version, (unsigned long) MULLE_OBJC_RUNTIME_VERSION);
      return( 1);
   }

   if( MULLE_OBJC_RUNTIME_LOAD_VERSION != JIT_LOAD_VERSION)
   {
      fprintf( stderr, "JIT/Runtime header load version mismatch (%u %lu)\n",
         JIT_LOAD_VERSION, (unsigned long) MULLE_OBJC_RUNTIME_LOAD_VERSION);
      return( 1);
   }
   return 0;
}


