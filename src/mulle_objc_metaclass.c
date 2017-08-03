//
//  mulle_objc_metaclass.c
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/07
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
#include "mulle_objc_metaclass.h"

#include "mulle_objc_class.h"

#include <mulle_concurrent/mulle_concurrent.h>
#include <mulle_allocator/mulle_allocator.h>


# pragma mark - sanity check

int   mulle_objc_metaclass_is_sane( struct _mulle_objc_metaclass *meta)
{
   if( ! meta)
   {
      errno = EINVAL;
      return( 0);
   }

   return( _mulle_objc_class_is_sane( &meta->base));
}


#pragma mark - walker

mulle_objc_walkcommand_t
   mulle_objc_metaclass_walk( struct _mulle_objc_metaclass   *meta,
                               enum mulle_objc_walkpointertype_t  type,
                               mulle_objc_walkcallback_t   callback,
                               void *parent,
                               void *userinfo)
{
   mulle_objc_walkcommand_t     cmd;

   cmd = mulle_objc_class_walk( _mulle_objc_metaclass_as_class( meta), type, callback, parent, userinfo);
   return( cmd);
}

