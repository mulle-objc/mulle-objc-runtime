//
//  mulle_objc_walktypes.h
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/08.
//  Copyright (c) 2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2017 Codeon GmbH.
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
#ifndef mulle_objc_walktypes_h__
#define mulle_objc_walktypes_h__

#include "include.h"

#include "mulle-objc-uniqueid.h"


typedef enum
{
   mulle_objc_walk_error        = -1,

   mulle_objc_walk_ok           = 0,
   mulle_objc_walk_dont_descend = 1,  // skip descent
   mulle_objc_walk_done         = 2,
   mulle_objc_walk_cancel       = 3
} mulle_objc_walkcommand_t;


// rename this
enum mulle_objc_walkpointertype_t
{
   mulle_objc_walkpointer_is_universe,
   mulle_objc_walkpointer_is_classpair,
   mulle_objc_walkpointer_is_infraclass,
   mulle_objc_walkpointer_is_metaclass,
   mulle_objc_walkpointer_is_category,
   mulle_objc_walkpointer_is_protocol,
   mulle_objc_walkpointer_is_method,
   mulle_objc_walkpointer_is_property,
   mulle_objc_walkpointer_is_ivar
};


struct _mulle_objc_universe;

typedef mulle_objc_walkcommand_t
      (*mulle_objc_walkcallback_t)( struct _mulle_objc_universe *universe,
                                    void *p,
                                    enum mulle_objc_walkpointertype_t type,
                                    char *key,
                                    void *parent,
                                    void *userinfo);


struct _mulle_objc_class;
struct _mulle_objc_method;
struct _mulle_objc_methodlist;


struct _mulle_objc_methodparent
{
   struct _mulle_objc_class       *cls;
   struct _mulle_objc_methodlist  *list;
}; 


typedef   mulle_objc_walkcommand_t 
   (*mulle_objc_method_walkcallback_t)( struct _mulle_objc_method *,
                                        struct _mulle_objc_methodlist *,
                                        struct _mulle_objc_class *,
                                        void *);


static inline struct _mulle_objc_methodlist  *
   _mulle_objc_methodparent_get_methodlist( struct _mulle_objc_methodparent *parent)
{
   return( parent->list);
}


static inline struct _mulle_objc_class  *
   _mulle_objc_methodparent_get_class( struct _mulle_objc_methodparent *parent)
{
   return( parent->cls);
}


static inline int   mulle_objc_walkcommand_is_stopper( mulle_objc_walkcommand_t cmd)
{
   switch( cmd)
   {
   case mulle_objc_walk_error  :
   case mulle_objc_walk_done   :
   case mulle_objc_walk_cancel :
      return( 1);

   default :
      return( 0);
   }
}


struct _mulle_objc_property;
struct _mulle_objc_infraclass;

typedef mulle_objc_walkcommand_t
   (*mulle_objc_walkpropertiescallback_t)( struct _mulle_objc_property *,
                                           struct _mulle_objc_infraclass *,
                                           void *);

struct _mulle_objc_ivar;

typedef mulle_objc_walkcommand_t
   (*mulle_objc_walkivarscallback_t)( struct _mulle_objc_ivar *,
                                      struct _mulle_objc_infraclass *,
                                      void *);


struct _mulle_objc_classpair;

typedef mulle_objc_walkcommand_t
   (*mulle_objc_walkprotocolclassescallback_t)( struct _mulle_objc_infraclass *,
                                                struct _mulle_objc_classpair *,
                                                void *);

typedef mulle_objc_walkcommand_t
   (*mulle_objc_walkcategoryidscallback_t)( mulle_objc_categoryid_t,
                                            struct _mulle_objc_classpair *,
                                            void *);

typedef mulle_objc_walkcommand_t
   (*mulle_objc_walkprotocolidscallback_t)( mulle_objc_protocolid_t,
                                            struct _mulle_objc_classpair *,
                                            void *);   

struct _mulle_objc_protocol;

typedef mulle_objc_walkcommand_t
   (*mulle_objc_walkprotocolcallback_t)( struct _mulle_objc_protocol *,
                                         struct _mulle_objc_universe *universe,
                                         void *);

#endif /* mulle_objc_walktypes_h */
