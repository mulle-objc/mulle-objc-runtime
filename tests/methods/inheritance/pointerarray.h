//
//  pointerarray.h
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//

#ifndef pointerarray_h__
#define pointerarray_h__

#pragma mark -
#pragma mark _pointerarray, simple growing array all inlined

struct _pointerarray
{
   size_t     n;
   size_t     size;
   void       **pointers;
};


static inline struct _pointerarray  *_pointerarray_alloc( void *(*calloc)( size_t, size_t))
{
   return( (struct _pointerarray *) (*calloc)( 1, sizeof( struct _pointerarray)));
}


static int   _pointerarray_grow( struct _pointerarray *array, void *(*realloc)( void *, size_t))
{
   array->size = array->size * 2;
   if( array->size < 2)
      array->size = 2;
   
   array->pointers = (*realloc)( array->pointers, sizeof( void *) * array->size);
   if( ! array->pointers)
   {
      array->size = 0;
      return( -1);
   }
   
   return( 0);
}


static inline int   _pointerarray_add( struct _pointerarray *array, void  *pointer,
                                       void *(*realloc)( void *, size_t))
{
   if( array->n == array->size)
      if( _pointerarray_grow( array, realloc))
         return( -1);
   
   array->pointers[ array->n++] = pointer;
   return( 0);
}


static inline void  *_pointerarray_get( struct _pointerarray *array, unsigned int i)
{
   assert( array);
   assert( i < array->n);
   return( array->pointers[ i]);
}


static inline unsigned int   _pointerarray_index( struct _pointerarray *array, void *p)
{
   void       **curr;
   void       **sentinel;

   curr     = array->pointers;
   sentinel = &curr[ array->n];
   while( curr < sentinel)
   {
      if( *curr == p)
         return( (unsigned int) (curr - array->pointers));
      curr++;
   }
   return( (unsigned int) -1);
}


static inline void   _pointerarray_set( struct _pointerarray *array, unsigned int i, void *p)
{
   assert( array);
   assert( i < array->n);
   array->pointers[ i] = p;
}


static inline void  _pointerarray_done( struct _pointerarray *array, void (*free)( void *))
{
   (*free)( array->pointers);
}


static inline void  pointerarray_free( struct _pointerarray *array, void (*free)( void *))
{
   _pointerarray_done( array, free);
   (*free)( array);
}


struct _pointerarray_enumerator
{
   void   **curr;
   void   **sentinel;
};


static inline struct  _pointerarray_enumerator   _pointerarray_enumerate( struct _pointerarray *array)
{
   struct _pointerarray_enumerator   rover;
   
   rover.curr     = &array->pointers[ 0];
   rover.sentinel = &rover.curr[ array->n];
   
   assert( rover.sentinel >= rover.curr);
   
   return( rover);
}


static inline void   *_pointerarray_enumerator_next( struct _pointerarray_enumerator *rover)
{
   return( rover->curr < rover->sentinel ? *rover->curr++ : (void *) -1);
}


static inline void  _pointerarray_enumerator_done( struct _pointerarray_enumerator *rover)
{
}

#endif
