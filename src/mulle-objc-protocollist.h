//
//  mulle_objc_protocollist.h
//  mulle-objc-runtime-universe
//
//  Created by Nat! on 30.05.17
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
#ifndef mulle_objc_protocollist_h__
#define mulle_objc_protocollist_h__

#include "include.h"

#include "mulle-objc-protocol.h"
#include "mulle-objc-walktypes.h"

#include <assert.h>
#include <stddef.h>


struct _mulle_objc_universe;

struct _mulle_objc_protocollist
{
   unsigned int                   n_protocols;
   struct _mulle_objc_protocol    protocols[ 1];
};


static inline size_t   mulle_objc_sizeof_protocollist( unsigned int n_protocols)
{
   return( sizeof( struct _mulle_objc_protocollist) + (n_protocols - 1) * sizeof( struct _mulle_objc_protocol));
}


MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_protocollist_sort( struct _mulle_objc_protocollist *list);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_protocol  *
   _mulle_objc_protocollist_find( struct _mulle_objc_protocollist *list,
                                           mulle_objc_protocolid_t protocolid);

static inline struct _mulle_objc_protocol  *_mulle_objc_protocollist_search( struct _mulle_objc_protocollist *list,
                                                                                    mulle_objc_protocolid_t protocolid)
{
   return( _mulle_objc_protocol_bsearch( list->protocols, list->n_protocols, protocolid));
}


static inline struct _mulle_objc_protocol  *_mulle_objc_protocollist_search_smart( struct _mulle_objc_protocollist *list,
                                                                                   mulle_objc_protocolid_t protocolid)
{
   if( list->n_protocols >= 14) // 14 is a researched value (i7)
      return( _mulle_objc_protocollist_search( list, protocolid));
   return( _mulle_objc_protocollist_find( list, protocolid));
}


# pragma mark - Enumerator

struct _mulle_objc_protocollistenumerator
{
   struct _mulle_objc_protocol   *sentinel;
   struct _mulle_objc_protocol   *protocol;
};


static inline struct  _mulle_objc_protocollistenumerator
   _mulle_objc_protocollist_enumerate( struct _mulle_objc_protocollist *list)
{
   struct _mulle_objc_protocollistenumerator   rover;

   rover.protocol = &list->protocols[ 0];
   rover.sentinel = &list->protocols[ list->n_protocols];

   assert( rover.sentinel >= rover.protocol);

   return( rover);
}


static inline struct _mulle_objc_protocol   *
   _mulle_objc_protocollistenumerator_next( struct _mulle_objc_protocollistenumerator *rover)
{
   return( rover->protocol < rover->sentinel ? rover->protocol++ : 0);
}


static inline void  _mulle_objc_protocollistenumerator_done( struct _mulle_objc_protocollistenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


# pragma mark - Protocol Walker

//
// supply cls and userinfo for callback, the cls is kinda ugly,
// but it's easier this way (no need to reorganize userinfo)
//
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_walkcommand_t
   _mulle_objc_protocollist_walk( struct _mulle_objc_protocollist *list,
                                  mulle_objc_walkprotocolcallback_t f,
                                  struct _mulle_objc_universe *universe,
                                  void *userinfo);

# pragma mark - protocollist API

static inline mulle_objc_walkcommand_t
   mulle_objc_protocollist_walk( struct _mulle_objc_protocollist *list,
                                 mulle_objc_walkprotocolcallback_t f,
                                 struct _mulle_objc_universe *universe,
                                 void *userinfo)
{
   if( ! list || ! f)
      return( -1);
   return( _mulle_objc_protocollist_walk( list, f, universe, userinfo));
}


static inline struct  _mulle_objc_protocollistenumerator
   mulle_objc_protocollist_enumerate( struct _mulle_objc_protocollist *list)
{
   struct _mulle_objc_protocollistenumerator   rover;

   if( ! list)
   {
      rover.protocol   = NULL;
      rover.sentinel = NULL;
      return( rover);
   }
   return( _mulle_objc_protocollist_enumerate( list));
}


static inline struct _mulle_objc_protocol   *
   mulle_objc_protocollistenumerator_next( struct _mulle_objc_protocollistenumerator *rover)
{
   return( rover ? _mulle_objc_protocollistenumerator_next( rover) : NULL);
}


static inline void
   mulle_objc_protocollistenumerator_done( struct _mulle_objc_protocollistenumerator *rover)
{
   if( rover)
      _mulle_objc_protocollistenumerator_done( rover);
}


#endif /* mulle_objc_protocollist_h */
