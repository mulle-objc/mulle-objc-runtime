//
//  main.c
//  mulle-msg-send
//
//  Created by Nat! on 15/11/14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-atomicpointer.h>
#include <stdio.h>


int main( int argc, char *argv[])
{
   char   buf[ 128];
   void   *pointer;

   mulle_objc_sprintf_functionpointer( buf, (void *) 0x12345678);
   sscanf( buf, "%p", &pointer);

   if( pointer != (void *) 0x12345678)
   {
      fprintf( stderr, "fail with %p\n", pointer);
      return( 1);
   }
   return( 0);
}

