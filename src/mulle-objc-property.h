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

#include "include.h"

#include "mulle-objc-uniqueid.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>


// encode characters in the property signature
enum mulle_objc_property_attribute
{
   mulle_objc_property_attribute_adder        = '+',
   mulle_objc_property_attribute_class        = 'K',
   mulle_objc_property_attribute_container    = 'K',
   mulle_objc_property_attribute_copy         = 'C',
   mulle_objc_property_attribute_dynamic      = 'D',
   mulle_objc_property_attribute_getter       = 'G',
   mulle_objc_property_attribute_ivar         = 'V',
   mulle_objc_property_attribute_nonatomic    = 'N',
   mulle_objc_property_attribute_observable   = 'O',
   mulle_objc_property_attribute_readonly     = 'R',
   mulle_objc_property_attribute_relationship = '>',
   mulle_objc_property_attribute_remover      = '-',
   mulle_objc_property_attribute_retain       = '&',
   mulle_objc_property_attribute_serializable = 'E',
   mulle_objc_property_attribute_setter       = 'S'
};


#define mulle_objc_property_attribute_adder_string          "+"
#define mulle_objc_property_attribute_container_string      "K"
#define mulle_objc_property_attribute_copy_string           "C"
#define mulle_objc_property_attribute_dynamic_string        "D"
#define mulle_objc_property_attribute_getter_string         "G"
#define mulle_objc_property_attribute_ivar_string           "V"
#define mulle_objc_property_attribute_nonatomic_string      "N"
#define mulle_objc_property_attribute_observable_string     "O"
#define mulle_objc_property_attribute_readonly_string       "R"
#define mulle_objc_property_attribute_relationship_string   ">"
#define mulle_objc_property_attribute_remover_string        "-"
#define mulle_objc_property_attribute_retain_string         "&"
#define mulle_objc_property_attribute_serializable_string   "E"
#define mulle_objc_property_attribute_setter_string         "S"


enum
{
   _mulle_objc_property_readonly          = 0x00001,
//   _mulle_objc_property_assign            = 0x00002,
//   _mulle_objc_property_readwrite         = 0x00004,
   _mulle_objc_property_retain            = 0x00008,
   _mulle_objc_property_copy              = 0x00010,
//   _mulle_objc_property_nullability       = 0x00020,
//   _mulle_objc_property_null_resettable   = 0x00040,
   _mulle_objc_property_nonnull           = 0x00080,
//   _mulle_objc_property_nullable          = 0x00100,
   _mulle_objc_property_class             = 0x00200,  // unused
   _mulle_objc_property_dynamic           = 0x00400,  // implement via forward:
   _mulle_objc_property_nonserializable   = 0x00800,  // exclude from NSCoder
   _mulle_objc_property_container         = 0x01000,  // --addToValue:/-removeFromValue:
   _mulle_objc_property_relationship      = 0x02000,  // -willReadRelationship:
   _mulle_objc_property_observable        = 0x04000,  // -willChange

   _mulle_objc_property_setterclear       = 0x10000,  // setter clear
   _mulle_objc_property_autoreleaseclear  = 0x20000   // autorelease clear
};


// properties have no descriptor (why ?)

struct _mulle_objc_property
{
   mulle_objc_propertyid_t    propertyid;
   mulle_objc_propertyid_t    ivarid;
   char                       *name;
   char                       *signature;  // hmmm...
   mulle_objc_methodid_t      getter;
   mulle_objc_methodid_t      setter;
   mulle_objc_methodid_t      adder;
   mulle_objc_methodid_t      remover;
   uint32_t                   bits;        // for pointers/objects
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


static inline mulle_objc_methodid_t    _mulle_objc_property_get_adder( struct _mulle_objc_property *property)
{
   return( property->adder);
}


static inline mulle_objc_methodid_t    _mulle_objc_property_get_remover( struct _mulle_objc_property *property)
{
   return( property->remover);
}


static inline uint32_t   _mulle_objc_property_get_bits( struct _mulle_objc_property *property)
{
   return( property->bits);
}


static inline uint32_t   _mulle_objc_property_is_dynamic( struct _mulle_objc_property *property)
{
   return( (property->bits & _mulle_objc_property_dynamic) ? 1 : 0);
}


static inline uint32_t   _mulle_objc_property_is_readonly( struct _mulle_objc_property *property)
{
   return( (property->bits & _mulle_objc_property_readonly) ? 1 : 0);
}


static inline uint32_t   _mulle_objc_property_is_observable( struct _mulle_objc_property *property)
{
   return( (property->bits & _mulle_objc_property_observable) ? 1 : 0);
}


static inline uint32_t   _mulle_objc_property_is_relationship( struct _mulle_objc_property *property)
{
   return( (property->bits & _mulle_objc_property_relationship) ? 1 : 0);
}


static inline uint32_t   _mulle_objc_property_is_container( struct _mulle_objc_property *property)
{
   return( (property->bits & _mulle_objc_property_container) ? 1 : 0);
}


static inline uint32_t   _mulle_objc_property_is_nonserializable( struct _mulle_objc_property *property)
{
   return( (property->bits & _mulle_objc_property_nonserializable) ? 1 : 0);
}


// todo: fix this naming strangenesss
MULLE_OBJC_RUNTIME_GLOBAL
char   *_mulle_objc_property_signature_find_type( struct _mulle_objc_property *property, char type);

MULLE_OBJC_RUNTIME_GLOBAL
char   *_mulle_objc_propertysignature_next_types( char *s, char *types);


# pragma mark - bsearch

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_property   *
   _mulle_objc_property_bsearch( struct _mulle_objc_property *buf,
                                 unsigned int n,
                                 mulle_objc_propertyid_t search);


static inline struct _mulle_objc_property   *
   mulle_objc_property_bsearch( struct _mulle_objc_property *buf,
                                unsigned int n,
                                mulle_objc_propertyid_t search)
{
   if( ! buf || (int) n <= 0)
      return( NULL);

   return( _mulle_objc_property_bsearch( buf, n, search));
}

# pragma mark - qsort

MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_property_compare( struct _mulle_objc_property *a,
                                   struct _mulle_objc_property *b);

MULLE_OBJC_RUNTIME_GLOBAL
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


static inline mulle_objc_methodid_t    mulle_objc_property_get_adder( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_adder( property) : MULLE_OBJC_NO_METHODID);
}


static inline mulle_objc_methodid_t    mulle_objc_property_get_remover( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_remover( property) : MULLE_OBJC_NO_METHODID);
}


static inline uint32_t    mulle_objc_property_get_bits( struct _mulle_objc_property *property)
{
   return( property ? _mulle_objc_property_get_bits( property) : MULLE_OBJC_NO_METHODID);
}


#endif
