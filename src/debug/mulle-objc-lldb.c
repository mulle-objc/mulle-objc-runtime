//
//  mulle_objc_lldb.c
//  mulle-objc-runtime-universe
//
//  Created by Nat! on 18.05.17.
//  Copyright © 2017 Mulle kybernetiK.
//  Copyright © 2017 Codeon GmbH.
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
#include "mulle-objc-lldb.h"

#include "mulle-objc-call.h"
#include "mulle-objc-class.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-method.h"
#include "mulle-objc-universe.h"


# pragma mark - lldb support

// DO NOT RENAME THESE FUNCTIONS, mulle-lldb looks for them
mulle_objc_implementation_t
   mulle_objc_lldb_lookup_implementation( void *obj,
                                                mulle_objc_methodid_t methodid,
                                                void *cls_or_classid,
                                                int is_classid,
                                                int is_meta,
                                                int debug)
{
   struct _mulle_objc_class            *cls;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_infraclass       *found;
   struct _mulle_objc_class            *call_cls;
   mulle_objc_implementation_t   imp;

   if( debug)
      fprintf( stderr, "lookup %p %08x %p (%d)\n", obj, methodid, cls_or_classid, is_classid);

   if( ! obj || mulle_objc_uniqueid_is_sane( MULLE_OBJC_NO_METHODID))
      return( 0);

   // ensure class init
   cls  = is_meta ? obj : _mulle_objc_object_get_isa( obj);

   // call "-class" so class initializes.. But WHY ??
   // if( ! _mulle_objc_metaclass_get_state_bit( meta, MULLE_OBJC_METACLASS_INITIALIZE_DONE))
   //   mulle_objc_object_call( cls, MULLE_OBJC_CLASS_METHODID, NULL);

   if( is_classid)
   {
      universe  = _mulle_objc_class_get_universe( cls);
      found    = _mulle_objc_universe_unfailingfastlookup_infraclass( universe,
                                                                        (mulle_objc_classid_t) (uintptr_t) cls_or_classid);
      if( is_meta)
         call_cls = _mulle_objc_metaclass_as_class( _mulle_objc_infraclass_get_metaclass( found));
      else
         call_cls = _mulle_objc_infraclass_as_class( found);
   }
   else
      call_cls = cls_or_classid;

   imp = _mulle_objc_class_noncachinglookup_implementation_no_forward( call_cls, methodid);
   if( debug)
   {
      char   buf[ s_mulle_objc_sprintf_functionpointer_buffer];

      mulle_objc_sprintf_functionpointer( buf, (mulle_functionpointer_t) imp);
      fprintf( stderr, "resolved to %p -> %s\n", call_cls, buf);
   }
   return( imp);
}


struct _mulle_objc_descriptor  *
   mulle_objc_lldb_lookup_descriptor_by_name( char *name)
{
   mulle_objc_methodid_t        methodid;
   struct _mulle_objc_universe   *universe;

   methodid = mulle_objc_uniqueid_from_string( name);
   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_lookup_descriptor( universe, methodid));
}


// this is supposed to crash!
void   mulle_objc_lldb_check_object( void *obj, mulle_objc_methodid_t sel)
{
   struct _mulle_objc_class  *cls;

   if( ! obj)
      return;

   cls = _mulle_objc_object_get_isa( obj);
   strlen( cls->name);    // try to crash here

   if( ! _mulle_objc_class_lookup_implementation_no_forward( cls, sel))
      *((volatile int *)0) = '1848'; // force crash
}


void   *mulle_objc_lldb_get_dangerous_classstorage_pointer( void)
{
   struct _mulle_objc_universe   *universe;
   struct mulle_concurrent_hashmap *map;

   universe = mulle_objc_get_universe();
   map     = &universe->classtable;
   return( _mulle_atomic_pointer_read( &map->next_storage.pointer));
}

