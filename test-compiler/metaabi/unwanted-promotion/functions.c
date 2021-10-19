//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


//
// TODO: move these to other .c file and link
//
void   print_double( void *self, mulle_objc_methodid_t _cmd, double x)
{
   printf( "%.2f\n", x);
}


void   print_char( void *self, mulle_objc_methodid_t _cmd, char x)
{
   printf( "%d\n", x);
}


