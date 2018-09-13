//
//  mulle_objc.h
//  mulle-objc-runtime
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
#ifndef mulle_objc_runtime_h__
#define mulle_objc_runtime_h__


// catch this early
#if ! defined( __MULLE_OBJC_TPS__) && ! defined( __MULLE_OBJC_NO_TPS__)
# error "Use the mulle-clang compiler to compile mulle-objc code (or define either __MULLE_OBJC_TPS__ or __MULLE_OBJC_NO_TPS__)"
#endif
#if defined( __MULLE_OBJC_TPS__) && defined( __MULLE_OBJC_NO_TPS__)
# error "Use the mulle-clang compiler to compile mulle-objc code (do not define both __MULLE_OBJC_TPS__ and __MULLE_OBJC_NO_TPS__)"
#endif

#if ! defined( __MULLE_OBJC_TRT__) && ! defined( __MULLE_OBJC_NO_TRT__)
# error "Use the mulle-clang compiler to compile mulle-objc code (or define either __MULLE_OBJC_TRT__ or __MULLE_OBJC_NO_TRT__)"
#endif
#if defined( __MULLE_OBJC_TRT__) && defined( __MULLE_OBJC_NO_TRT__)
# error "Use the mulle-clang compiler to compile mulle-objc code (do not define both __MULLE_OBJC_TRT__ and __MULLE_OBJC_NO_TRT__)"
#endif

#if ! defined( __MULLE_OBJC_FMC__) && ! defined( __MULLE_OBJC_NO_FMC__)
# error "Use the mulle-clang 5.0.0.0 (or newer) compiler to compile mulle-objc code (or define either __MULLE_OBJC_FMC__ or __MULLE_OBJC_NO_FMC__)"
#endif
#if defined( __MULLE_OBJC_FMC__) && defined( __MULLE_OBJC_NO_FMC__)
# error "Use the mulle-clang 5.0.0.0 (or newer) compiler to compile mulle-objc code (do not define both __MULLE_OBJC_FMC__ and __MULLE_OBJC_NO_FMC__)"
#endif

#include "include.h"

#include "mulle-objc-atomicpointer.h"
#include "mulle-objc-builtin.h"
#include "mulle-objc-call.h"
#include "mulle-objc-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-class-convenience.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-class-struct.h"
#include "mulle-objc-fastclasstable.h"
#include "mulle-objc-fastmethodtable.h"
#include "mulle-objc-fnv1.h"
#include "mulle-objc-fnv1a.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-ivar.h"
#include "mulle-objc-ivarlist.h"
#include "mulle-objc-kvccache.h"
#include "mulle-objc-load.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-method.h"
#include "mulle-objc-methodidconstants.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-object.h"
#include "mulle-objc-objectheader.h"
#include "mulle-objc-object-convenience.h"
#include "mulle-objc-property.h"
#include "mulle-objc-propertylist.h"
#include "mulle-objc-retain-release.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-universe-global.h"
#include "mulle-objc-universe-struct.h"
#include "mulle-objc-signature.h"
#include "mulle-objc-super.h"
#include "mulle-objc-taggedpointer.h"
#include "mulle-objc-try-catch-finally.h"
#include "mulle-objc-uniqueid.h"
#include "mulle-objc-uniqueidarray.h"
#include "mulle-objc-version.h"
#include "mulle-objc-walktypes.h"

#include "mulle-objc-universe-class.h"


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

