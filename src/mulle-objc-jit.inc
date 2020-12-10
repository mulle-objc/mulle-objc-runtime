//
//  mulle-objc-compiler.h
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/08.
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2018 Codeon GmbH.
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

//
// the mulle-clang compiler needs a small subset of the mulle-objc-runtime
// for operation. For JIT operation use of the full mulle-objc-runtime
// header is preferred. But if locating the proper header is impossible
// (say during deployment) but the content of the file at the top of your
// JIT code.
//
#define MULLE_OBJC_RUNTIME_VERSION_MAJOR  0   // max 511
#define MULLE_OBJC_RUNTIME_VERSION_MINOR  18  // max 1023
#define MULLE_OBJC_RUNTIME_VERSION_PATCH  0   // max 255

//
// up the number if binary loads are incompatible
// this is read and checked against by the compiler
//
#define MULLE_OBJC_RUNTIME_LOAD_VERSION   16


#ifndef __TEST_VERSION

// #define __MULLE_OBJC_TPS__
// //#define __MULLE_OBJC_NO_TPS__
// #define __MULLE_OBJC_FCS__
// //#define __MULLE_OBJC_NO_FCS__


//
// Above is sufficient to get -O0 working.
// The following lines are are de-inlining mulle-objc-runtime functions for
// compatibility with -O1, -O2, -O3.
//

struct _mulle_objc_infraclass;

// preferably defined elsewhre
typedef int    mulle_objc_uniqueid_t;  // ** MUST BE 32 bit (on the stack) **


static inline struct _mulle_objc_infraclass *
   mulle_objc_lookup_infraclass_inline_nofail( mulle_objc_uniqueid_t universeid,
                                              mulle_objc_uniqueid_t classid)
{
	extern struct _mulle_objc_infraclass *
		mulle_objc_lookup_infraclass_nofail( mulle_objc_uniqueid_t universeid, mulle_objc_uniqueid_t classid);

   return( mulle_objc_lookup_infraclass_nofail( universeid, classid));
}


static inline void  *
	mulle_objc_object_call_inline( void *obj,
                                                 mulle_objc_uniqueid_t methodid,
                                                 void *parameter)
{
	extern void  *mulle_objc_object_call( void *obj,
                                        mulle_objc_uniqueid_t methodid,
                                        void *parameter);

	return( mulle_objc_object_call( obj, methodid, parameter));
}


static inline void  *
   mulle_objc_object_supercall_inline( void *obj,
                                       mulle_objc_uniqueid_t methodid,
                                       void *parameter,
                                       mulle_objc_uniqueid_t superid)
{
	extern void  *
	   mulle_objc_object_supercall( void *obj,
	                                 mulle_objc_uniqueid_t methodid,
	                                 void *parameter,
	                                 mulle_objc_uniqueid_t superid);

	return( mulle_objc_object_supercall( obj, methodid, parameter, superid));
}


static inline void   *mulle_objc_object_retain_inline( void *obj)
{
	extern void   *mulle_objc_object_retain( void *obj);

	return( mulle_objc_object_retain( obj));
}


static inline void   mulle_objc_object_release_inline( void *obj)
{
	extern void   mulle_objc_object_release( void *obj);

	mulle_objc_object_release( obj);
}


/*
-O1

    mulle_objc_lookup_infraclass_nofail
    mulle_objc_object_call_inline_partial
    mulle_objc_object_supercall_inline_partial
*/

static inline void  *
	mulle_objc_object_call_inline_partial( void *obj,
                                             mulle_objc_uniqueid_t methodid,
                                             void *parameter)
{
	extern void  *mulle_objc_object_call( void *obj,
                                         mulle_objc_uniqueid_t methodid,
                                         void *parameter);

	return( mulle_objc_object_call( obj, methodid, parameter));
}


static inline void   *
   mulle_objc_object_supercall_inline_partial( void *obj,
                                              mulle_objc_uniqueid_t methodid,
                                              void *parameter,
                                              mulle_objc_uniqueid_t superid)
{
	extern void   *
   	mulle_objc_object_supercall( void *obj,
      	                          mulle_objc_uniqueid_t methodid,
         	                        void *parameter,
            	                    mulle_objc_uniqueid_t superid);

	return( mulle_objc_object_supercall( obj, methodid, parameter, superid));
}

#endif
