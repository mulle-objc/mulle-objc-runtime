//
//  mulle_objc_fastclasstable.h
//  mulle-objc-runtime
//
//  Created by Nat! on 01.02.16.
//  Copyright (c) 2016 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
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
#ifndef mulle_objc_fastclasstable_h__
#define mulle_objc_fastclasstable_h__

#include "include.h"

#include "mulle-objc-atomicpointer.h"

#include <assert.h>


struct _mulle_objc_infraclass;


// change compiler if you change this
#define MULLE_OBJC_S_FASTCLASSES   64

//
// Adding more classes is relatively inexpensive, as this table is
// only wired to the universe (vs. the fastmethodtable is on every class)
// but of course you need to recompile the universe (and up the version)
// Parts of this file should be auto generated, when MULLE_OBJC_S_FASTCLASSES
// is increased, but we ain't there yet.
//
struct _mulle_objc_fastclasstable
{
   union _mulle_objc_atomicclasspointer_t   classes[ MULLE_OBJC_S_FASTCLASSES];
};



static inline struct _mulle_objc_infraclass *
    mulle_objc_fastclasstable_get_infraclass( struct _mulle_objc_fastclasstable *table,
                                              unsigned int i)
{
   return( (struct _mulle_objc_infraclass *) _mulle_atomic_pointer_read( &table->classes[ i].pointer));
}



MULLE_C_CONST_NONNULL_RETURN MULLE_C_STATIC_ALWAYS_INLINE
struct _mulle_objc_infraclass  *
     mulle_objc_fastclasstable_get_infraclass_nofail( struct _mulle_objc_fastclasstable *table,
                                                      unsigned int i)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   int   mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls);
   struct _mulle_objc_infraclass   *infra;

   infra = (struct _mulle_objc_infraclass *) _mulle_atomic_pointer_read( &table->classes[ i].pointer);

   assert( infra);
   assert( mulle_objc_class_is_current_thread_registered( (void *) infra));
   return( infra);
}


//
// this is a lookup function mapping classid to an index
// the compiler will use this function for the specified classes to
// make a direct lookup of the class. This is useful for classes, that
// are known to have many classcalls (like NSString f.e.)
//
// By default there are no CLASSIDs configured. That is the job of the
// Foundation. The Foundation should leave at least 50% free for user
// classes. IMO. Obvious candidates are all the small clases, NSData, NSString
// an obvious non-candidate would be NSFileManager.
//
MULLE_C_CONST_RETURN MULLE_C_STATIC_ALWAYS_INLINE
int   mulle_objc_get_fastclasstable_index( mulle_objc_classid_t classid)
{
   switch( classid)
   {
#ifdef MULLE_OBJC_FASTCLASSHASH_0
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_0 ) : return( 0);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_1
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_1 ) : return( 1);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_2
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_2 ) : return( 2);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_3
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_3 ) : return( 3);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_4
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_4 ) : return( 4);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_5
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_5 ) : return( 5);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_6
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_6 ) : return( 6);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_7
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_7 ) : return( 7);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_8
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_8 ) : return( 8);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_9
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_9 ) : return( 9);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_10
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_10) : return( 10);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_11
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_11) : return( 11);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_12
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_12) : return( 12);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_13
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_13) : return( 13);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_14
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_14) : return( 14);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_15
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_15) : return( 15);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_16
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_16) : return( 16);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_17
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_17) : return( 17);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_18
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_18) : return( 18);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_19
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_19) : return( 19);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_20
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_20) : return( 20);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_21
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_21) : return( 21);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_22
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_22) : return( 22);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_23
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_23) : return( 23);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_24
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_24) : return( 24);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_25
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_25) : return( 25);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_26
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_26) : return( 26);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_27
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_27) : return( 27);
#endif

#ifdef MULLE_OBJC_FASTCLASSHASH_28
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_28) : return( 28);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_29
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_29) : return( 29);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_30
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_30) : return( 30);
#endif
#ifdef MULLE_OBJC_FASTCLASSHASH_31
      case MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_31) : return( 31);
#endif
   }
   return( -1);
}

#endif /* mulle_objc_fastclasstable_h */
