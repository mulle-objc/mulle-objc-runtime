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
#include <math.h>
#include <float.h>

//
//    Sign bit: 1 bit
//    Exponent: 11 bits
//    Significand precision: 53 bits (52 explicitly stored)
static int   exponents[] =
{
   0x7FF,
   0x3FF,
   0x1FF,
   0x0FF,
   1024,
   -1023,
   -1022,
   0 - 1023,
   1 - 1023,
   2 - 1023,
   4 - 1023,
   8 - 1023,
   32 - 1023,
   64 - 1023,
   128 - 1023,
   256 - 1023,
   512 - 1023
};


static uint64_t   mantissas[] =
{
   0,
   1,
   0x7FFFFFFFFFFFF,
   0xFFFFFFFFFFFFF
};


int   main( int argc, const char * argv[])
{
   union
   {
      uint64_t   v;
      double     d;
   } c;
   int        sign;
   int        exponent_i;
   int        exponent;
   int        mantissa_i;
   uint64_t   mantissa;
   void       *tp;
   int        is_tp;
   double     before;

   for( sign = 0; sign <= 1; sign++)
   {
     for( exponent_i = 0; exponent_i < sizeof( exponents) / sizeof( exponents[ 0]); exponent_i++)
     {
        exponent = exponents[ exponent_i];
        for( mantissa_i = 0; mantissa_i < sizeof( mantissas) / sizeof( mantissas[ 0]); mantissa_i++)
        {
            mantissa = mantissas[ mantissa_i];

            c.v  = sign ? 0x8000000000000000 : 0;
            c.v += (uint64_t) (exponent & 0x7FF) << 52;
            c.v += mantissa;

            before = c.d;

            is_tp = mulle_objc_taggedpointer_is_valid_double_value( c.d);
            mulle_printf( "%d,%lld=%016llx (%g) -> %s",
                          exponent_i,
                          mantissa_i,
                          (unsigned long long) c.v,
                          c.d,
                          is_tp ? "TPS" : "NO ");
            if( is_tp)
            {
               tp  = mulle_objc_create_double_taggedpointer( c.d, 0x5);
               c.d = mulle_objc_taggedpointer_get_double_value( tp);
               mulle_printf( " -> %p -> %016llx (%g)",
                             tp,
                             (unsigned long long) c.v,
                             c.d);
               if( c.d != before &&
                   ! (isnan( c.d) && isnan( before)) &&
                   ! (isinf( c.d) && isinf( before)))
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

