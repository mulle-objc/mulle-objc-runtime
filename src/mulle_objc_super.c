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

#include "mulle_objc_super.h"

#include "mulle_objc_uniqueid.h"
#include <string.h>
#include <errno.h>


int   mulle_objc_super_is_sane( struct _mulle_objc_super *p)
{
   if( ! p)
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! mulle_objc_uniqueid_is_sane_string( p->superid, p->name))
      return( 0);

   if( ! mulle_objc_uniqueid_is_sane( p->classid) ||
       ! mulle_objc_uniqueid_is_sane( p->methodid))
   {
      errno = EINVAL;
      return( 0);
   }

   return( 1);
}
