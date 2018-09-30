//
//  main.m
//  test-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//

// code makes no sense without assert
#ifdef NDEBUG
#undef NDEBUG
#endif

#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-test-allocator/mulle-test-allocator.h>

#include "test_simple_inheritance.h"
#include "pointerarray.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


#pragma mark -
#pragma mark exceptions

static void   test_fail( char *format, va_list args)          __attribute__ ((noreturn));
static void   test_inconsistency( char *format, va_list args) __attribute__ ((noreturn));

static void   test_fail( char *format, va_list args)
{
   fprintf( stderr, "general exception:");
   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");
   abort();
}


static void   test_inconsistency( char *format, va_list args)
{
   fprintf( stderr, "inconsistency exception:");
   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");
   abort();
}


#pragma mark -
#pragma mark reset universe between tests

static void   reset_universe()
{
   struct _mulle_objc_universe   *universe;

   mulle_objc_release_universe();

   universe = mulle_objc_register_universe();

   // tests were written at an earlier time...
   universe->classdefaults.inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS;
}


extern void   test_simple_inheritance();
extern void   test_category_inheritance();
extern void   test_class_simple();
extern void   test_protocol_inheritance( void);
extern void   test_message_sending( void);
extern void   test_message_forwarding1( void);
extern void   test_message_forwarding2( void);
extern void   test_retain_release( void);
extern void   test_method( void);


int main( int argc, const char * argv[])
{
   reset_universe();

   fprintf( stderr, "simple\n");
   test_class_simple();
   reset_universe();

   fprintf( stderr, "simple_inheritance\n");
   test_simple_inheritance();
   reset_universe();

   fprintf( stderr, "category_inheritance\n");
   test_category_inheritance();
   reset_universe();

   fprintf( stderr, "protocol_inheritance\n");
   test_protocol_inheritance();
   reset_universe();

   fprintf( stderr, "method\n");
   test_method();
   reset_universe();

   fprintf( stderr, "message_sending\n");
   test_message_sending();
   reset_universe();

   fprintf( stderr, "message_forwarding1\n");
   test_message_forwarding1();
   reset_universe();

   fprintf( stderr, "message_forwarding2\n");
   test_message_forwarding2();
   reset_universe();

   fprintf( stderr, "retain_releas\n");
   test_retain_release();

   reset_universe();

   return( 0);
}
