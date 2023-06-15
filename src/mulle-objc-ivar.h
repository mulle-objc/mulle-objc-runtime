//
//  mulle_objc_ivar.h
//  mulle-objc-runtime
//
//  Created by Nat! on 11.08.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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
#ifndef mulle_objc_ivar_h__
#define mulle_objc_ivar_h__

#include "include.h"

#include "mulle-objc-uniqueid.h"

#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>


struct _mulle_objc_ivardescriptor
{
   mulle_objc_ivarid_t   ivarid;
   char                  *name;
   char                  *signature;
};


//
// this is used for properties and regular ivars
//
struct _mulle_objc_ivar
{
   struct _mulle_objc_ivardescriptor   descriptor;
   int                                 offset;  // if == -1 : hashed access (future)
};


# pragma mark - ivar petty accessors

static inline char   *_mulle_objc_ivar_get_name( struct _mulle_objc_ivar *ivar)
{
   return( ivar->descriptor.name);
}


static inline char   *_mulle_objc_ivar_get_signature( struct _mulle_objc_ivar *ivar)
{
   return( ivar->descriptor.signature);
}


static inline mulle_objc_ivarid_t  _mulle_objc_ivar_get_ivarid( struct _mulle_objc_ivar *ivar)
{
   return( ivar->descriptor.ivarid);
}


static inline int    _mulle_objc_ivar_get_offset( struct _mulle_objc_ivar *ivar)
{
   return( ivar->offset);
}


# pragma mark - bsearch

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_ivar   *_mulle_objc_ivar_bsearch( struct _mulle_objc_ivar *buf,
                                                     unsigned int n,
                                                     mulle_objc_ivarid_t search);

static inline struct _mulle_objc_ivar   *mulle_objc_ivar_bsearch( struct _mulle_objc_ivar *buf,
                                                                  unsigned int n,
                                                                  mulle_objc_ivarid_t search)
{
   if( ! buf || (int) n <= 0)
      return( NULL);

   return( _mulle_objc_ivar_bsearch( buf, n, search));
}


# pragma mark - qsort

MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_ivar_compare( struct _mulle_objc_ivar *a, struct _mulle_objc_ivar *b);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_ivar_sort( struct _mulle_objc_ivar *ivars, unsigned int n);


# pragma mark - API

static inline char   *mulle_objc_ivar_get_name( struct _mulle_objc_ivar *ivar)
{
   return( ivar ? _mulle_objc_ivar_get_name( ivar) : NULL);
}


static inline char   *mulle_objc_ivar_get_signature( struct _mulle_objc_ivar *ivar)
{
   return( ivar ? _mulle_objc_ivar_get_signature( ivar) : NULL);
}


static inline int    mulle_objc_ivar_get_offset( struct _mulle_objc_ivar *ivar)
{
   return( ivar ? _mulle_objc_ivar_get_offset( ivar) : -1);
}


#endif
