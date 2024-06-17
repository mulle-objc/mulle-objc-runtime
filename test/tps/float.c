//
//  main.m
//  test-md5
//
//  Created by Nat! on 26.02.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_TPS__
# define __MULLE_OBJC_FCS__
# ifdef DEBUG
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif


#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <float.h>
#include <math.h>

//
//    Sign bit: 1 bit
//    Exponent: 11 bits
//    Significand precision: 53 bits (52 explicitly stored)
static int   exponents[] =
{
   0,
   0xFF,
   0x7F,
   0x3F,
   0x1F,
   0x0F,
   128,
   -127,
   -126,
   0 - 127,
   1 - 127,
   2 - 127,
   4 - 127,
   8 - 127,
   32 - 127,
   64 - 127,
   128 - 127,
};


static uint32_t   mantissas[] =
{
   0,
   1,
   0x3FFFFF,
   0x7FFFFF
};

int   main( int argc, const char * argv[])
{
   union
   {
      uint32_t   v;
      float      f;
   } c;
   int             sign;
   int             exponent_i;
   int             exponent;
   int             mantissa_i;
   uint32_t        mantissa;
   void            *tp;
   int             is_tp;
   float           before;

   for( sign = 0; sign <= 1; sign++)
   {
     for( exponent_i = 0; exponent_i < sizeof( exponents) / sizeof( exponents[ 0]); exponent_i++)
     {
        exponent = exponents[ exponent_i];
        for( mantissa_i = 0; mantissa_i < sizeof( mantissas) / sizeof( mantissas[ 0]); mantissa_i++)
        {
            mantissa = mantissas[ mantissa_i];

            c.v = sign ? 0x80000000 : 0;
            c.v += (uint32_t) (exponent & 0xFF) << 23;
            c.v += mantissa;

            before = c.f;

            is_tp = mulle_objc_taggedpointer_is_valid_float_value( c.f);
            mulle_printf( "%d,%ld=%08lx (%g) -> %s",
                          exponent_i, 
                          (long) mantissa_i,
                          (unsigned long) c.v,
                          c.f,
                          is_tp ? "TPS" : "NO ");
            if( is_tp)
            {
               tp  = mulle_objc_create_float_taggedpointer( c.f, 0x3);
               c.f = mulle_objc_taggedpointer_get_float_value( tp);
               mulle_printf( " -> %p -> %08lx (%g)",
                             tp,
                             (unsigned long) c.v,
                             c.f);
               if( c.f != before &&
                   ! (isnan( c.f) && isnan( before)) &&
                   ! (isinf( c.f) && isinf( before)))
               {
                  mulle_printf( "*FAIL*");
               }
            }
            mulle_printf( "\n");
        }
      }
   }
   return( 0);
}

