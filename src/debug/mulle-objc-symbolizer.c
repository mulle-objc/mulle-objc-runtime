//
//  mulle_objc_symbolizer.c
//  mulle-objc-runtime
//
//  Created by Nat! on 9/4/19.
//  Copyright (c) 2019 Nat! - Mulle kybernetiK.
//  Copyright (c) 2019 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#include "mulle-objc-symbolizer.h"

#include "mulle-objc-class.h"
#include "mulle-objc-method.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-walktypes.h"


#include <errno.h>
#include <stdlib.h>
#include <stdint.h>


//
// https://reviews.llvm.org/D46155
// TODO: research why C-Standard has a problem with that
//
#pragma clang diagnostic ignored "-Wordered-compare-function-pointers"

struct mmc
{
   struct _mulle_objc_method       *method;
   struct _mulle_objc_methodlist   *list;
   struct _mulle_objc_class        *class;
};


static int   mmc_snprint( struct mmc  *p,
                          struct _mulle_objc_universe *universe,
                          char *buf,
                          size_t len)
{
   mulle_objc_categoryid_t   categoryid;
   char                      tmp[ 16];
   char                      *s;

   // it's not a category ?
   if( ! p->list->owner)
      return( snprintf( buf, len, "%c[%s %s]",
                                  _mulle_objc_class_is_metaclass( p->class) ? '+' : '-',
                                  _mulle_objc_class_get_name( p->class),
                                  _mulle_objc_method_get_name( p->method)));

   categoryid = (mulle_objc_categoryid_t) (uintptr_t) p->list->owner;
   s          = _mulle_objc_universe_search_hashstring( universe, categoryid);
   if( ! s)
   {
      sprintf( tmp, "%08x", categoryid);
      s = tmp;
   }

   return( snprintf( buf, len, "%c[%s( %s) %s]",
                               _mulle_objc_class_is_metaclass( p->class) ? '+' : '-',
                               _mulle_objc_class_get_name( p->class),
                               s,
                               _mulle_objc_method_get_name( p->method)));
}


struct mmcarray
{
   struct mmc   *_values;
   size_t       _count;
   size_t       _size;
};


static inline void   mmcarray_init( struct mmcarray *array,
                                    unsigned int  capacity)
{
   array->_size      = 0;
   array->_count     = 0;

   if( capacity)
   {
      if( capacity < 2)
         capacity = 2;

      array->_values = mulle_allocator_calloc( &mulle_stdlib_allocator,
                                               capacity,
                                               sizeof( struct mmc));
      array->_size = capacity;
   }
}


static inline void  mmcarray_done( struct mmcarray *array)
{
   mulle_allocator_free( &mulle_stdlib_allocator, array->_values);
}



# pragma mark -
# pragma mark petty accessors

static inline unsigned int
   mmcarray_get_count( struct mmcarray *array)
{
   return( array->_count);
}



# pragma mark -
# pragma mark operations

static int   mmcarray_grow( struct mmcarray *array)
{
   unsigned int   new_size;

   new_size = array->_size * 2;
   if( new_size < 2)
      new_size = 2;

   array->_values = mulle_allocator_realloc( &mulle_stdlib_allocator,
                                             array->_values,
                                             sizeof( struct mmc) * new_size);
   memset( &array->_values[ array->_size], 0, sizeof( struct mmc) * (new_size - array->_size));
   array->_size = new_size;

   return( 0);
}


static inline int   mmcarray_add( struct mmcarray *array,
                                  struct mmc *p)
{
   if( array->_count == array->_size)
      if( mmcarray_grow( array))
      {
         assert( 0);
         abort();
      }

   array->_values[ array->_count++] = *p;

   return( 0);
}


static int   mmc_compare_imps( struct mmc *p_a,
                               struct mmc *p_b)
{
   struct _mulle_objc_method     *a = p_a->method;
   struct _mulle_objc_method     *b = p_b->method;
   mulle_objc_implementation_t   aImp;
   mulle_objc_implementation_t   bImp;

   aImp = _mulle_objc_method_get_implementation( a);
   bImp = _mulle_objc_method_get_implementation( b);
   if( (void *) aImp < (void *) bImp)
      return( -1);
   return( aImp == bImp ? 0 : +1);
}


static inline void
   mmcarray_sort( struct mmcarray *array,
                  int (*f)( struct mmc *, struct mmc *))
{
   // also sorts zeroes
   qsort( array->_values,
          array->_count,     // n
          sizeof( struct mmc),  // sizeof
          (int (*)( const void *, const void *)) f);
}


struct mmc *
   mmcarray_find_method( struct mmcarray *array,
                         struct _mulle_objc_method *method)
{
   struct mmc   *p;
   struct mmc   *sentinel;

   p        = array->_values;
   sentinel = &p[ array->_count];
   while( p < sentinel)
   {
      if( p->method == method)
         return( p);
      ++p;
   }
   return( NULL);
}


struct mmc *
   mmcarray_find_nearestimp( struct mmcarray *array,
                             mulle_objc_implementation_t search)
{
   int                           first;
   int                           last;
   int                           middle;
   int                           n;
   struct mmc                    *p;
   mulle_objc_implementation_t   imp;
   mulle_objc_implementation_t   nextimp;

   n      = array->_count;
   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p   = &array->_values[ middle];
      imp = _mulle_objc_method_get_implementation( p->method);
      if( (void *) search < (void *) imp)
      {
         last = middle - 1;
      }
      else
      {
         if( middle + 1 == n)
            return( p);

         nextimp = _mulle_objc_method_get_implementation( array->_values[ middle + 1].method);
         if( (void *) search < (void *) nextimp)
            return( p);

         first = middle + 1;
      }

      middle = (first + last) / 2;
   }

   return( NULL);
}


void   mmcarray_csvdump( struct mmcarray *array,
                         struct _mulle_objc_universe *universe,
                         FILE *fp)
{
   struct mmc                *p;
   struct mmc                *sentinel;
   auto char                 buf[ 256];
   char                      *s;
   mulle_objc_categoryid_t   categoryid;

   p        = array->_values;
   sentinel = &p[ array->_count];
   while( p < sentinel)
   {
      fprintf( fp, "%08x", _mulle_objc_class_get_classid( p->class));
      fprintf( fp, ";%s", _mulle_objc_class_get_name( p->class));

      categoryid = (int) (intptr_t) p->list->owner;
      fprintf( fp, ";%08x", categoryid);
      s = NULL;
      if( categoryid)
         s = _mulle_objc_universe_search_hashstring( universe, categoryid);
      fprintf( fp, ";%s", s ? s : "");

      if( mmc_snprint( p, universe, buf, sizeof( buf)) < 0)
         fprintf( fp, "%08x;%p\n",
                      _mulle_objc_method_get_methodid( p->method),
                      (void *) _mulle_objc_method_get_implementation( p->method));
      else
         fprintf( fp, "%s;%p\n", buf, (void *) _mulle_objc_method_get_implementation( p->method));
      ++p;
   }
}


#if 0
static void   mmc_print( struct mmc  *p,
                         struct _mulle_objc_universe *universe,
                         FILE *fp)
{
   mulle_objc_categoryid_t   categoryid;
   char                      buf[ 16];
   char                      *s;

   // it's not a category ?
   if( ! p->list->owner)
   {
      fprintf( fp, "%c[%s %s]", _mulle_objc_class_is_metaclass( p->class) ? '+' : '-',
                                  _mulle_objc_class_get_name( p->class),
                                  _mulle_objc_method_get_name( p->method));
      return;
   }

   categoryid = (mulle_objc_categoryid_t) (uintptr_t) p->list->owner;
   s          = _mulle_objc_universe_search_hashstring( universe, categoryid);
   if( ! s)
   {
      sprintf( buf, "%08x", categoryid);
      s = buf;
   }

   fprintf( fp, "%c[%s( %s) %s]", _mulle_objc_class_is_metaclass( p->class) ? '+' : '-',
                                  _mulle_objc_class_get_name( p->class),
                                  s,
                                  _mulle_objc_method_get_name( p->method));
}
#endif


/*
 *
 */

static inline struct mmcarray *
   _mulle_objc_symbolizer_get_array( struct mulle_objc_symbolizer *p)
{
   return( (struct mmcarray *) &p->array);
}



static mulle_objc_walkcommand_t
      callback( struct _mulle_objc_universe *universe,
                void *p,
                enum mulle_objc_walkpointertype_t type,
                char *key,
                void *parent,
                void *userinfo)
{
   struct mmc   mmc;

   if( type != mulle_objc_walkpointer_is_method)
      return( mulle_objc_walk_ok);

   mmc.method = p;
   mmc.list   = _mulle_objc_methodparent_get_methodlist( parent);
   mmc.class  = _mulle_objc_methodparent_get_class( parent);

   assert( ! mmcarray_find_method( userinfo, p));

   mmcarray_add( userinfo, &mmc);

   return( mulle_objc_walk_ok);
}


void   _mulle_objc_symbolizer_init( struct mulle_objc_symbolizer *p,
                                    struct _mulle_objc_universe *universe)
{
   struct mmcarray   *array;

   p->universe = universe;

   array = _mulle_objc_symbolizer_get_array( p);
   mmcarray_init( array, 2048);
   mulle_objc_universe_walk( universe, callback, array);
   mmcarray_sort( array, mmc_compare_imps);
}


struct mulle_objc_symbolizer *
   _mulle_objc_symbolizer_create( struct _mulle_objc_universe *universe)
{
   struct mulle_allocator         *allocator;
   struct mulle_objc_symbolizer   *p;

   //
   // because we can't have the test-allocator intercepting our calls
   // we use "mulle_stdlib_allocator" malloc here.
   //
   p = mulle_allocator_malloc( &mulle_stdlib_allocator,
                               sizeof( struct mulle_objc_symbolizer));
   _mulle_objc_symbolizer_init( p, universe);
   return( p);
}


void   _mulle_objc_symbolizer_done( struct mulle_objc_symbolizer *p)
{
   mmcarray_done( _mulle_objc_symbolizer_get_array( p));
}


void   mulle_objc_symbolizer_destroy( struct mulle_objc_symbolizer *p)
{
   if( ! p)
      return;

   _mulle_objc_symbolizer_done( p);
   mulle_allocator_free( &mulle_stdlib_allocator, p);
}



int   mulle_objc_symbolizer_snprint( struct mulle_objc_symbolizer *p,
                                     void *address,
                                     size_t max,
                                     char *buf,
                                     size_t len)
{
   int                           written;
   int                           written2;
   struct mmc                    *q;
   ptrdiff_t                     diff;
   mulle_objc_implementation_t   imp;

   if( ! p)
   {
      errno = EINVAL;
      return( -1);
   }

   q = mmcarray_find_nearestimp( _mulle_objc_symbolizer_get_array( p),
                                 (mulle_objc_implementation_t) address);
   if( ! q)
      return( 0);

   imp  = _mulle_objc_method_get_implementation( q->method);
   diff = (intptr_t) address - (intptr_t) imp;
   if( diff > max)
      return( 0);

   written = mmc_snprint( q, p->universe, buf, len);
   if( diff && written >= 0)
   {
      written2 = snprintf( &buf[ written], len - written, "+0x%lx", (long) diff);
      if( written2 < 0)
         return( written2);
      written += written2;
   }
   return( written);
}




void   _mulle_objc_universe_csvdump_methods( struct _mulle_objc_universe *universe,
                                             FILE *fp)
{
   struct mulle_objc_symbolizer   *p;

   p = _mulle_objc_symbolizer_create( universe);
   mmcarray_csvdump( _mulle_objc_symbolizer_get_array( p), universe, fp);
   mulle_objc_symbolizer_destroy( p);
}

