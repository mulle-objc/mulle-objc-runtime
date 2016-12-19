//
//  main.c
//  research-cache-spread
//
//  Created by Nat! on 16.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USE_MERSENNE  0

#ifdef USE_MERSENNE
# include "mt19937ar.h"
#endif


static int   compare_unsigned_int( unsigned int  *a, unsigned int *b)
{
   if( *a > *b)
      return( -1);
   if( *a < *b)
      return( 1);
   return( 0);
}


int main(int argc, const char * argv[])
{
   unsigned int   n_slots;
   unsigned int   n_entries;
   unsigned int   *slots;
   unsigned int   value;
   unsigned int   i;
   unsigned int   n;
   unsigned int   count;
   
   if( argc != 3)
   {
      errno = EINVAL;
      perror( "argc:");
      return( -1);
   }
   
   n_slots   = atoi( argv[ 1]);     // ex. 3000
   n_entries = atoi( argv[ 2]);     // ex. 1000
   
   if( n_slots <= n_entries)
   {
      errno = EINVAL;
      perror( "argv:");
      return( -1);
   }
   
   slots = calloc( n_slots, sizeof( unsigned int));
   if( ! slots)
   {
      perror( "calloc:");
      return( -1);
   }
   
   srand( (unsigned) time( NULL));
#if USE_MERSENNE
   {
      unsigned long   init[4] ={ rand(), rand(), rand(), rand() };
      
      init_by_array (init, 4);
   }
#define rand()   genrand_int32()
#endif
   
   // generate random numbers, figure out how many
   // clashes there are to be expected
   
   for( i = 0; i < n_entries; i++)
   {
      value = rand() % n_slots;
      slots[ value]++;
   }
   
   qsort( slots, n_slots, sizeof( unsigned int), (void *) compare_unsigned_int);
   
   
   /* auswerten */
   value = slots[ 0];
   count = 1;
   
   for( i = 0; i < n_entries; i++)
   {
      if( slots[ i] != value)
      {
         printf( "%.1f%%: %u\n", ((count * 100.0 + n_entries / 2) / n_entries), value);
         value = slots[ i];
         count = 1;
      }
      if( ! slots[ i])
         break;
      ++count;
   }
   
   return 0;
}
