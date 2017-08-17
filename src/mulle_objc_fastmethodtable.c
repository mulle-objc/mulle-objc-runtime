//
//  mulle_objc_fast_call.c
//  mulle-objc-runtime
//
//  Created by Nat! on 26/10/15.
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
#include "mulle_objc_fastmethodtable.h"

#ifdef __MULLE_OBJC_FMC__

#include "mulle_objc_call.h"
#include "mulle_objc_class.h"
#include "mulle_objc_object.h"
#include "mulle_objc_universe.h"


static void   *_mulle_objc_object_handle_fastmethodtablefault( void *obj,
                                                               mulle_objc_methodid_t methodid,
                                                               void *param,
                                                               unsigned int index)
{
   struct _mulle_objc_class            *cls;
   struct _mulle_objc_universe          *universe;
   mulle_objc_implementation_t   imp;

   // don't cache it
   cls     = _mulle_objc_object_get_isa( obj);
   universe = _mulle_objc_class_get_universe( cls);

   // looking up methods, should be thread safe
   {
      imp = _mulle_objc_class_noncachinglookup_implementation( cls, methodid);

      if( universe->debug.trace.method_call)
      {
         // trace but don't cache it
         mulle_objc_class_trace_method_call( cls, methodid, obj, param, imp);
      }
      else
         _mulle_atomic_pointer_write( &cls->vtab.methods[ index].pointer, imp);
   }

   // go through class call to hit +initialize
   return( (cls->call)( obj, methodid, param, cls));
}



/* sorry, but it was just too much copy/paste */
#define faulthandler( x) \
   static void   *_mulle_objc_fastmethodtablefaulthandler_ ## x( void *obj, mulle_objc_methodid_t sel, void *param)    \
{                        \
   return( _mulle_objc_object_handle_fastmethodtablefault( obj, sel, param, (x)));  \
}


faulthandler( 0)
faulthandler( 1)
faulthandler( 2)
faulthandler( 3)
faulthandler( 4)
faulthandler( 5)
faulthandler( 6)
faulthandler( 7)
faulthandler( 8)
faulthandler( 9)
faulthandler( 10)
faulthandler( 11)
faulthandler( 12)
faulthandler( 13)
faulthandler( 14)
faulthandler( 15)
faulthandler( 16)
faulthandler( 17)
faulthandler( 18)
faulthandler( 19)
faulthandler( 20)
faulthandler( 21)
faulthandler( 22)
faulthandler( 23)


void   _mulle_objc_fastmethodtable_init( struct _mulle_objc_fastmethodtable *table)
{
   _mulle_atomic_pointer_write( &table->methods[ 0].pointer, _mulle_objc_fastmethodtablefaulthandler_0);
   _mulle_atomic_pointer_write( &table->methods[ 1].pointer, _mulle_objc_fastmethodtablefaulthandler_1);
   _mulle_atomic_pointer_write( &table->methods[ 2].pointer, _mulle_objc_fastmethodtablefaulthandler_2);
   _mulle_atomic_pointer_write( &table->methods[ 3].pointer, _mulle_objc_fastmethodtablefaulthandler_3);
   _mulle_atomic_pointer_write( &table->methods[ 4].pointer, _mulle_objc_fastmethodtablefaulthandler_4);
   _mulle_atomic_pointer_write( &table->methods[ 5].pointer, _mulle_objc_fastmethodtablefaulthandler_5);
   _mulle_atomic_pointer_write( &table->methods[ 6].pointer, _mulle_objc_fastmethodtablefaulthandler_6);
   _mulle_atomic_pointer_write( &table->methods[ 7].pointer, _mulle_objc_fastmethodtablefaulthandler_7);

   _mulle_atomic_pointer_write( &table->methods[ 8].pointer, _mulle_objc_fastmethodtablefaulthandler_8);
   _mulle_atomic_pointer_write( &table->methods[ 9].pointer, _mulle_objc_fastmethodtablefaulthandler_9);
   _mulle_atomic_pointer_write( &table->methods[10].pointer, _mulle_objc_fastmethodtablefaulthandler_10);
   _mulle_atomic_pointer_write( &table->methods[11].pointer, _mulle_objc_fastmethodtablefaulthandler_11);
   _mulle_atomic_pointer_write( &table->methods[12].pointer, _mulle_objc_fastmethodtablefaulthandler_12);
   _mulle_atomic_pointer_write( &table->methods[13].pointer, _mulle_objc_fastmethodtablefaulthandler_13);
   _mulle_atomic_pointer_write( &table->methods[14].pointer, _mulle_objc_fastmethodtablefaulthandler_14);
   _mulle_atomic_pointer_write( &table->methods[15].pointer, _mulle_objc_fastmethodtablefaulthandler_15);

   _mulle_atomic_pointer_write( &table->methods[16].pointer, _mulle_objc_fastmethodtablefaulthandler_16);
   _mulle_atomic_pointer_write( &table->methods[17].pointer, _mulle_objc_fastmethodtablefaulthandler_17);
   _mulle_atomic_pointer_write( &table->methods[18].pointer, _mulle_objc_fastmethodtablefaulthandler_18);
   _mulle_atomic_pointer_write( &table->methods[19].pointer, _mulle_objc_fastmethodtablefaulthandler_19);
   _mulle_atomic_pointer_write( &table->methods[20].pointer, _mulle_objc_fastmethodtablefaulthandler_20);
   _mulle_atomic_pointer_write( &table->methods[21].pointer, _mulle_objc_fastmethodtablefaulthandler_21);
   _mulle_atomic_pointer_write( &table->methods[22].pointer, _mulle_objc_fastmethodtablefaulthandler_22);
   _mulle_atomic_pointer_write( &table->methods[23].pointer, _mulle_objc_fastmethodtablefaulthandler_23);
}

#endif
