//
//  mulle_objc_property.h
//  mulle-objc-runtime
//
//  Created by Nat! on 05.03.15.
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
#ifndef mulle_objc_property_h__
#define mulle_objc_property_h__

#include "mulle_objc_uniqueid.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>


// properties have no descriptor (why ?)

struct _mulle_objc_property
{
   mulle_objc_propertyid_t    propertyid;
   mulle_objc_ivarid_t        ivarid;      // name prefixed with _
   char                       *name;
   char                       *signature;  // hmmm...
   mulle_objc_methodid_t      getter;
   mulle_objc_methodid_t      setter;
   mulle_objc_methodid_t      clearer;     // for pointers/objects
};


# pragma mark - property petty accessors

static inline char   *_mulle_objc_property_get_name( struct _mulle_objc_property *property)
{
   return( property->name);
}


static inline char   *_mulle_objc_property_get_signature( struct _mulle_objc_property *property)
{
   return( property->signature);
}


static inline mulle_objc_propertyid_t  _mulle_objc_property_get_propertyid( struct _mulle_objc_property *property)
{
   return( property->propertyid);
}


// can be 0 if @dynamic
static inline mulle_objc_ivarid_t  _mulle_objc_property_get_ivarid( struct _mulle_objc_property *property)
{
   return( property->ivarid);
}


static inline mulle_objc_methodid_t    _mulle_objc_property_get_getter( struct _mulle_objc_property *property)
{
   return( property->getter);
}


static inline mulle_objc_methodid_t    _mulle_objc_property_get_setter( struct _mulle_objc_property *property)
{
   return( property->setter);
}


// todo: fix this naming strangenesss
char   *_mulle_objc_property_signature_find_type( struct _mulle_objc_property *property, char type);
char   *_mulle_objc_propertysignature_next_types( char *s, char *types);


# pragma mark - bsearch

struct _mulle_objc_property   *_mulle_objc_property_bsearch( struct _mulle_objc_property *buf,
                                                             unsigned int n,
                                                             mulle_objc_propertyid_t search);


static inline struct _mulle_objc_property   *mulle_objc_property_bsearch( struct _mulle_objc_property *buf,
                                                                          unsigned int n,
                                                                          mulle_objc_propertyid_t search)
{
   if( ! buf || (int) n <= 0)
      return( NULL);

   return( _mulle_objc_property_bsearch( buf, n, search));
}

# pragma mark - qsort

int  _mulle_objc_property_compare( struct _mulle_objc_property *a,
                                   struct _mulle_objc_property *b);

void   mulle_objc_property_sort( struct _mulle_objc_property *properties,
                                 unsigned int n);


# pragma mark - API

static inline char   *mulle_objc_property_get_name( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_name( property) : NULL);
}


static inline char   *mulle_objc_property_get_signature( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_signature( property) : NULL);
}


static inline mulle_objc_propertyid_t  mulle_objc_property_get_propertyid( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_propertyid( property) : MULLE_OBJC_NO_PROPERTYID);
}


static inline mulle_objc_methodid_t    mulle_objc_property_get_getter( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_getter( property) : MULLE_OBJC_NO_METHODID);
}


static inline mulle_objc_methodid_t    mulle_objc_property_get_setter( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_setter( property) : MULLE_OBJC_NO_METHODID);
}


#endif
