//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <stdint.h>


int main(int argc, const char * argv[])
{
   char   c;

   c = 127;
   // check that third argument is promoted correctly to int
   print_char( (void *) 0, (uint32_t) 0x1, c);
   print_double( (void *) 0, (uint32_t) 0x2, 18.48);

   return( 0);
}


