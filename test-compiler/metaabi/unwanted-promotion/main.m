//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <stdint.h>


extern void   print_double( void *self, uint32_t _cmd, double x);
extern void   print_char( void *self, uint32_t _cmd, char x);


int main(int argc, const char * argv[])
{
   char   c;

   c = 127;

   // check that third argument is promoted correctly to int
   // ** don't put in prototypes!! ***
   print_char( (void *) 0, (uint32_t) 0x1, c);
   print_double( (void *) 0, (uint32_t) 0x2, 18.48);

   return( 0);
}


