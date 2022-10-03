//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 31.10.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//


__attribute__((noinline))
void  *callWithAB( double a, double b)
{
   void   *p = 0;
   return( &p);
}



int   main( int argc, const char * argv[])
{
   void   *osp;
   void   *sp;
   int    i;

   sp = 0;

   for( i = 0; i < 2; i++)
   {
      osp = sp;
      sp  = callWithAB( 1.0, 2.0);
      if( osp == (void *) 0)
         continue;
      if( sp != osp)
         return( 1);
   }
   return( 0);
}

