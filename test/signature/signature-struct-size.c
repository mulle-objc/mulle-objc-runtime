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


struct test_0
{
   char    c;
};

struct test_1
{
   char    c;
   short   s;
};

struct test_2
{
   char    c;
   int     i;
};

struct test_3
{
   char    c;
   double  d;
};

struct test_4
{
   char    c;
   char    c2;
};


struct test_5
{
   short   s;
   double  d;
};


struct test_6
{
   struct test_0  x;
   struct test_0  y;
};


struct test_7
{
   short   s;
   char    c;
};

struct test_8
{
   int     i;
   char    c;
};

struct test_9
{
   double  d;
   char    c;
};


struct test_10
{
   double  d;
   short   s;
};


struct test_11
{
   struct test_1   x1;
   struct test_2   x2;
   struct test_7   x7;
   struct test_8   x8;
};


static struct test
{
   char          *signature;
   size_t        size;
   unsigned int  align;
} tests[] =
{
   { .signature = "{?=c}",  .size = sizeof( struct test_0), .align = alignof( struct test_0) },
   { .signature = "{?=cs}", .size = sizeof( struct test_1), .align = alignof( struct test_1) },
   { .signature = "{?=ci}", .size = sizeof( struct test_2), .align = alignof( struct test_2) },
   { .signature = "{?=cd}", .size = sizeof( struct test_3), .align = alignof( struct test_3) },
   { .signature = "{?=cc}", .size = sizeof( struct test_4), .align = alignof( struct test_4) },
   { .signature = "{?=sd}", .size = sizeof( struct test_5), .align = alignof( struct test_5) },
   { .signature = "{?={?=c}{?=c}}", .size = sizeof( struct test_6), .align = alignof( struct test_6) },
   { .signature = "{?=sc}", .size = sizeof( struct test_7), .align = alignof( struct test_7) },
   { .signature = "{?=ic}", .size = sizeof( struct test_8), .align = alignof( struct test_8) },
   { .signature = "{?=dc}", .size = sizeof( struct test_9), .align = alignof( struct test_9) },
   { .signature = "{?=ds}", .size = sizeof( struct test_10), .align = alignof( struct test_10) },
   { .signature = "{?={?=cs}{?=ci}{?=sc}{?=ic}}", .size = sizeof( struct test_11), .align = alignof( struct test_11) },
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


