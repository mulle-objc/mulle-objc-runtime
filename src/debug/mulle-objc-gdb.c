//
//  mulle-objc-gdb.h
//  mulle-objc-runtime
//
//  Created by Nat! on 14.10.19
//  Copyright © 2019 Mulle kybernetiK. All rights reserved.
//  Copyright © 2019 Codeon GmbH. All rights reserved.
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
#ifndef mulle_objc_gdb_h__
#define mulle_objc_gdb_h__

#include "mulle-objc-gdb.h"

#include "mulle-objc-call.h"
#include "mulle-objc-class.h"
#include "mulle-objc-class-convenience.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-method.h"
#include "mulle-objc-retain-release.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-universe-global.h"

//
// used by the mulle-gdb debugger
//
struct _mulle_objc_class  *
   mulle_objc_gdb_lookup_class( char *name)
{
   struct _mulle_objc_universe     *universe;
   mulle_objc_classid_t            classid;
   struct _mulle_objc_infraclass   *infra;

   classid  = mulle_objc_uniqueid_from_string( name);
   universe = mulle_objc_global_inlineget_universe( MULLE_OBJC_DEFAULTUNIVERSEID);
   infra    = _mulle_objc_universe_lookup_infraclass( universe, classid);
   return( infra ? _mulle_objc_infraclass_as_class( infra) : NULL);
}


mulle_objc_methodid_t
   mulle_objc_gdb_lookup_selector( char *name)
{
   struct _mulle_objc_universe     *universe;
   mulle_objc_methodid_t           sel;
   struct _mulle_objc_descriptor   *desc;

   sel      = mulle_objc_uniqueid_from_string( name);
   universe = mulle_objc_global_inlineget_universe( MULLE_OBJC_DEFAULTUNIVERSEID);
   desc     = _mulle_objc_universe_lookup_descriptor( universe, sel);

   return( desc ? sel : MULLE_OBJC_NO_METHODID);
}



mulle_objc_implementation_t
   mulle_objc_gdb_lookup_implementation( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *class_or_superid,
                                         int calltype)

{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_infraclass   *super;
   mulle_objc_implementation_t     imp;
   mulle_objc_superid_t            superid;
   int                             olderrno;

   olderrno = errno;

   if( ! obj || mulle_objc_uniqueid_is_sane( MULLE_OBJC_NO_METHODID))
   {
      errno = olderrno;
      return( 0);
   }

   //
   // the resulting IMP is the one, the debugger will step "thru"
   // if we want to step into forwarded methods, we would have to do the
   // complete forward: resolution, which we can't at this level.
   //
   switch( calltype)
   {
   case 0 :
      cls = _mulle_objc_object_get_isa( obj);
      imp = _mulle_objc_class_lookup_implementation_nocache( cls, methodid);
      break;

   case 1 :
      imp = _mulle_objc_class_lookup_implementation_nocache( class_or_superid,
                                                             methodid);
      break;

      // doing this nofail, is bad. Tracing into [super forwardedMessage]
      // will bring grief
   case 2 :
      superid = (mulle_objc_superid_t) (uintptr_t) class_or_superid;
      imp     = _mulle_objc_object_inlinesuperlookup_implementation_nofail( obj,
                                                                            superid);
   }

   errno = olderrno;

   return( imp);
}


#endif
