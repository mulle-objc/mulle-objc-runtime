//
//  main.c
//  mulle-msg-send
//
//  Created by Nat! on 15/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle_objc_runtime/mulle_objc_runtime.h>
#include <stdio.h>


struct _foo
{
   int  a, b;
};


void   print_foo( struct _foo *_par)
{
   printf( "%d%d\n", _par->a, _par->b);
}


struct _baz
{
   struct _foo  x;
   int          a, b;
};


struct _foobar
{
   struct _foo  x;
   int          a, b;
};


void   print_foobar( struct _foobar *_par)
{
   print_foo( &_par->x);
   printf( "%d%d\n", _par->a, _par->b);
}



int main( int argc, char *argv[])
{
   struct _foo   tmp;

   print_foo( (void *) &(struct { int a; int b; }){ .a = 18, .b = 48 });

   tmp.a = 6;
   tmp.b = 66;
   print_foobar( (void *) &(struct { struct _foo a; int b; int c; }){ .a = tmp, .b = 18, .c = 48 });

   return( 0);
}

