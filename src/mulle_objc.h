//
//  mulle_objc.h
//  mulle-objc
//
//  Created by Nat! on 10.07.16.
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
#ifndef mulle_objc_h__
#define mulle_objc_h__


// catch this early
#if ! defined( __MULLE_OBJC_TPS__) && ! defined( __MULLE_OBJC_NO_TPS__)
# error "Use the mulle-clang compiler to compile mulle-objc code (or define either __MULLE_OBJC_TPS__ or __MULLE_OBJC_NO_TPS__)"
#endif

#if ! defined( __MULLE_OBJC_TRT__) && ! defined( __MULLE_OBJC_NO_TRT__)
# error "Use the mulle-clang compiler to compile mulle-objc code (or define either __MULLE_OBJC_TRT__ or __MULLE_OBJC_NO_TRT__)"
#endif

#include "mulle_objc_atomicpointer.h"
#include "mulle_objc_builtin.h"
#include "mulle_objc_call.h"
#include "mulle_objc_class.h"
#include "mulle_objc_classpair.h"
#include "mulle_objc_class_convenience.h"
#include "mulle_objc_class_struct.h"
#include "mulle_objc_fastclasstable.h"
#include "mulle_objc_fastmethodtable.h"
#include "mulle_objc_fnv1.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_ivar.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_kvccache.h"
#include "mulle_objc_load.h"
#include "mulle_objc_metaclass.h"
#include "mulle_objc_method.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_object.h"
#include "mulle_objc_objectheader.h"
#include "mulle_objc_object_convenience.h"
#include "mulle_objc_property.h"
#include "mulle_objc_propertylist.h"
#include "mulle_objc_retain_release.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_runtime_global.h"
#include "mulle_objc_runtime_struct.h"
#include "mulle_objc_signature.h"
#include "mulle_objc_taggedpointer.h"
#include "mulle_objc_try_catch_finally.h"
#include "mulle_objc_version.h"
#include "mulle_objc_walktypes.h"

#include "mulle_objc_class_runtime.h"

#include <mulle_aba/mulle_aba.h>
#include <mulle_allocator/mulle_allocator.h>
#include <mulle_concurrent/mulle_concurrent.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_vararg/mulle_vararg.h>

// add some functions to mulle-vararg for ObjC

#define mulle_vararg_count_ids( args, obj) \
   mulle_vararg_count_pointers( (args), (obj))
#define mulle_vararg_count_objects( args, obj) \
   mulle_vararg_count_pointers( (args), (obj))

#define mulle_vararg_next_id( args) \
   mulle_vararg_next_pointer( (args), id)
#define mulle_vararg_next_object( args, type) \
   mulle_vararg_next_pointer( (args), id)


#if MULLE_ABA_VERSION < ((1 << 20) | (4 << 8) | 0)
# error "mulle_aba is too old"
#endif
#if MULLE_ALLOCATOR_VERSION < ((2 << 20) | (1 << 8) | 0)
# error "mulle_allocator is too old"
#endif
#if MULLE_CONCURRENT_VERSION < ((1 << 20) | (3 << 8) | 0)
# error "mulle_concurrent is too old"
#endif
#if MULLE_THREAD_VERSION < ((3 << 20) | (2 << 8) | 0)
# error "mulle_thread is too old"
#endif
#if MULLE_VARARG_VERSION < ((0 << 20) | (5 << 8) | 0)
# error "mulle_vararg is too old"
#endif


#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>

#endif

