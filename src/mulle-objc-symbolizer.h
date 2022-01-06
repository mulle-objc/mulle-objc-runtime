//
//  mulle_objc_symbolizer.h
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
#ifndef mulle_objc_symbolizer_h__
#define mulle_objc_symbolizer_h__


#include <stddef.h>
#include <stdio.h>


struct _mulle_objc_universe;


struct mulle_objc_symbolizer
{
   struct _mulle_objc_universe  *universe;
   struct
   {
      void     *_values;
      size_t   _count;
      size_t   _size;
   } array;
};


MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void   _mulle_objc_symbolizer_init( struct mulle_objc_symbolizer *p,
                                   struct _mulle_objc_universe *universe);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
struct mulle_objc_symbolizer *
   _mulle_objc_symbolizer_create( struct _mulle_objc_universe *universe);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void   _mulle_objc_symbolizer_done( struct mulle_objc_symbolizer *p);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void   mulle_objc_symbolizer_destroy( struct mulle_objc_symbolizer *p);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
int   mulle_objc_symbolizer_snprint( struct mulle_objc_symbolizer *p,
                                     void *address,
                                     size_t max,
                                     char *buf,
                                     size_t len);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void   _mulle_objc_universe_csvdump_methods( struct _mulle_objc_universe *universe,
                                             FILE *fp);

static inline void   mulle_objc_symbolizer_done( struct mulle_objc_symbolizer *p)
{
   if( p)
      _mulle_objc_symbolizer_done( p);
}

#endif
