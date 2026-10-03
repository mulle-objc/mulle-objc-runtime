//
//  type-equal.c
//  mulle-objc-runtime
//
//  Created by Nat! on 2026.
//  Copyright (c) 2026 Mulle kybernetiK. All rights reserved.
//
#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_NO_TPS__
# define __MULLE_OBJC_FCS__
# if defined( DEBUG) || ! defined( __OPTIMIZE__)
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif



#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


static void   check( char *name, char *a, char *b)
{
   printf( "%-22s %d\n",
           name,
           _mulle_objc_type_is_equal_to_type( a, b));
}


int   main( void)
{
   // scalars ignore the (optional) trailing size, but not the type itself
   check( "scalar same", "i", "i");
   check( "scalar size opt", "i", "i4");
   check( "scalar different", "i", "q");

   // aggregates must compare structurally and ignore the optional size
   check( "struct same", "{mulle_range=QQ}", "{mulle_range=QQ}");
   check( "struct size opt", "{mulle_range=QQ}", "{mulle_range=QQ}16");
   check( "struct size opt rev", "{mulle_range=QQ}16", "{mulle_range=QQ}");
   check( "struct different", "{mulle_range=QQ}", "{mulle_range=II}");

   check( "nested same", "{a={b=i}d}8", "{a={b=i}d}8");
   check( "nested size opt", "{a={b=i}d}", "{a={b=i}d}8");
   check( "nested different", "{a={b=i}d}8", "{a={b=c}d}8");

   check( "array same", "[3i]", "[3i]12");
   check( "array different", "[3i]", "[4i]");

   check( "union same", "(i)", "(i)4");
   return( 0);
}
