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

#include <mulle_objc/mulle_objc.h>
#include <mulle_test_allocator/mulle_test_allocator.h>

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
#pragma mark reset runtime between tests

static void   reset_runtime()
{
   struct _mulle_objc_runtime   *runtime;

   mulle_objc_release_runtime();
   runtime = mulle_objc_get_or_create_runtime();

   // tests were written at an earlier time...
   runtime->classdefaults.inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS;
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
   reset_runtime();
#if 0
   test_class_simple();
   reset_runtime();
   test_simple_inheritance();

   reset_runtime();
   test_category_inheritance();

   reset_runtime();
   test_protocol_inheritance();

   reset_runtime();
   test_method();

   reset_runtime();
   test_message_sending();

   reset_runtime();
   test_message_forwarding1();

   reset_runtime();
   test_message_forwarding2();

   reset_runtime();
   test_retain_release();

   reset_runtime();
#endif
   return( 0);
}
