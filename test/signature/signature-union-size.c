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
# if defined( DEBUG) || ! defined( __OPTIMIZE__)
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif



#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


union test_0
{
   char    c;
};

union test_1
{
   char    c;
   short   s;
};

union test_2
{
   char    c;
   int     i;
};

union test_3
{
   char    c;
   double  d;
};

union test_4
{
   char    c;
   char    c2;
};


union test_5
{
   short   s;
   double  d;
};


union test_6
{
   union test_0  x;
   union test_0  y;
};


union test_7
{
   short   s;
   char    c;
};

union test_8
{
   int     i;
   char    c;
};

union test_9
{
   double  d;
   char    c;
};


union test_10
{
   double  d;
   short   s;
};


union test_11
{
   union test_1   x1;
   union test_2   x2;
   union test_7   x7;
   union test_8   x8;
};


static struct test
{
   char          *signature;
   size_t        size;
   unsigned int  align;
} tests[] =
{
   { "(?=c)",  sizeof( union test_0), alignof( union test_0) },
   { "(?=cs)", sizeof( union test_1), alignof( union test_1) },
   { "(?=ci)", sizeof( union test_2), alignof( union test_2) },
   { "(?=cd)", sizeof( union test_3), alignof( union test_3) },
   { "(?=cc)", sizeof( union test_4), alignof( union test_4) },
   { "(?=sd)", sizeof( union test_5), alignof( union test_5) },
   { "(?=(?=c)(?=c))", sizeof( union test_6), alignof( union test_6) },
   { "(?=sc)", sizeof( union test_7), alignof( union test_7) },
   { "(?=ic)", sizeof( union test_8), alignof( union test_8) },
   { "(?=dc)", sizeof( union test_9), alignof( union test_9) },
   { "(?=ds)", sizeof( union test_10), alignof( union test_10) },
   { "(?=(?=cs)(?=ci)(?=sc)(?=ic))", sizeof( union test_11), alignof( union test_11) },
   { 0 }
};



static void   dummy_callback( char *type, struct mulle_objc_typeinfo *info, void *userinfo)
{
}



static void   test( struct test *p)
{
   char                         *next;
   struct mulle_objc_typeinfo   type_info;

   printf( "type=\"%s\" (size=%zd, align=%u): ", p->signature, p->size, p->align);

   next = _mulle_objc_type_parse( p->signature,
                                  0,
                                  &type_info,
                                  _mulle_objc_signature_supply_scalar_typeinfo,
                                  dummy_callback,
                                  NULL);
   if( ! next)
   {
      printf( "SYNTAX ERROR IN SIGNATURE: \"%s\"\n", p->signature);
      return;
   }

   if( type_info.natural_size != p->size)
   {
      printf( "DIFFERENCE IN SIZE: \"%zd\" vs expected \"%zd\"\n",
                  type_info.natural_size, p->size);
      return;
   }
   if( type_info.natural_alignment != p->align)
   {
      printf( "DIFFERENCE IN ALIGN: \"%u\" vs expected \"%u\"\n",
                  type_info.natural_alignment, p->align);
      return;
   }
   printf( "OK\n");
}




int   main( int argc, const char * argv[])
{
   struct test  *p;

   for( p = tests; p->signature; p++)
   {
      test( p);
   }

   return 0;
}


