//
//  mulle_objc_protocollist.c
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
#include "mulle-objc-protocollist.h"

#include "mulle-objc-class.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-callqueue.h"

#include <assert.h>
#include <stdlib.h>


mulle_objc_walkcommand_t
   _mulle_objc_protocollist_walk( struct _mulle_objc_protocollist *list,
                                  mulle_objc_walkprotocolcallback_t f,
                                  struct _mulle_objc_universe *universe,
                                  void *userinfo)
{
   struct _mulle_objc_protocol   *sentinel;
   struct _mulle_objc_protocol   *p;
   int                           rval;

   assert( list);

   p        = &list->protocols[ 0];
   sentinel = &p[ list->n_protocols];

   while( p < sentinel)
   {
      if( rval = (*f)( p, universe, userinfo))
         return( rval);
      ++p;
   }

   return( 0);
}


struct _mulle_objc_protocol  *
   _mulle_objc_protocollist_find( struct _mulle_objc_protocollist *list,
                                           mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_protocol   *sentinel;
   struct _mulle_objc_protocol   *p;

   assert( list);
   assert( mulle_objc_uniqueid_is_sane( protocolid));

   p        = &list->protocols[ 0];
   sentinel = &p[ list->n_protocols];

   while( p < sentinel)
   {
      if( p->protocolid == protocolid)
         return( p);
      ++p;
   }

   return( 0);
}


void   mulle_objc_protocollist_sort( struct _mulle_objc_protocollist *list)
{
   if( list)
      mulle_objc_protocol_sort( list->protocols, list->n_protocols);
}

