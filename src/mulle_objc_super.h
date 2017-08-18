//
//  mulle_objc_super.h
//  mulle-objc-runtime
//
//  Created by Nat! on 12.08.17.
//  Copyright (c) 2017 Mulle kybernetiK. All rights reserved.
//  Copyright (c) 2017 Codeon GmbH.All rights reserved.
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
#ifndef mulle_objc_super_h__
#define mulle_objc_super_h__


#include "mulle_objc_uniqueid.h"
#include "mulle_objc_fnv1.h"
#include <string.h>


struct _mulle_objc_super
{
   mulle_objc_superid_t    superid;
   char                    *name;
   mulle_objc_classid_t    classid;
   mulle_objc_methodid_t   methodid;
};


# pragma mark - method petty accessors

static inline char   *_mulle_objc_super_get_name( struct _mulle_objc_super *p)
{
   return( p->name);
}


static inline mulle_objc_superid_t  _mulle_objc_super_get_superid( struct _mulle_objc_super *p)
{
   return( p->superid);
}


static inline mulle_objc_classid_t  _mulle_objc_super_get_classid( struct _mulle_objc_super *p)
{
   return( p->classid);
}


static inline mulle_objc_methodid_t  _mulle_objc_super_get_methodid( struct _mulle_objc_super *p)
{
   return( p->methodid);
}



struct _mulle_objc_superlist
{
   unsigned int                n_supers; 
   struct _mulle_objc_super    supers[ 1];
};


int   mulle_objc_super_is_sane( struct _mulle_objc_super *p);

#define MULLE_OBJC_SUPERSELECTOR_STRING( s_cls, s_method)       (s_cls ";" s_method)
#define MULLE_OBJC_OVERRIDDENSELECTOR_STRING( s_cls, s_method)  (s_cls " " s_method)


// used for overridden methods /future
static inline mulle_objc_superid_t
   mulle_objc_superid_from_classid_and_categoryname( mulle_objc_classid_t classid,
                                                     char *s)
{
   mulle_objc_superid_t   hash;

   hash = _mulle_objc_chained_fnv1_32( ";", 1, classid);
   hash = _mulle_objc_chained_fnv1_32( s, strlen( s), classid);
   return( hash);
}

#endif
