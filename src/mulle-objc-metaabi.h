//
//  mulle_objc_call.h
//  mulle-objc-runtime
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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
#ifndef mulle_objc_metaabi_h__
#define mulle_objc_metaabi_h__

/* if you need to "manually" call a MetaABI function with a _param block
   use mulle_objc_metaabi_param_block to generate it. DO NOT CALL IT
   `_param` THOUGH (triggers a compiler bug).

   ex.

   mulle_objc_metaabi_param_block( NSRange, NSUInteger)   param;

   param.p = NSMakeRange( 1, 1);
   mulle_objc_object_call( obj, sel, &param);
   return( param.rval);
*/

#define mulle_objc_void_5_pointers( size)  \
   ( ((size) + sizeof( void *[ 5]) - 1) / sizeof( void *[ 5]) )

#define mulle_objc_size_metaabi_param_block(  size) \
   ( sizeof( void *[ 5]) * mulle_objc_void_5_pointers( size) )

#define mulle_objc_metaabi_param_block( param_type, rval_type)                 \
   union                                                                       \
   {                                                                           \
      rval_type    r;                                                          \
      param_type   p;                                                          \
      void         *space[ 5][ sizeof( rval_type) > sizeof( param_type)        \
                         ?  mulle_objc_void_5_pointers( sizeof( rval_type))    \
                         :  mulle_objc_void_5_pointers( sizeof( param_type))]; \
   }

#define mulle_objc_metaabi_param_block_voidptr_return( param_type) \
   mulle_objc_metaabi_param_block( param_type, void *)

#define mulle_objc_metaabi_param_block_voidptr_parameter( return_type) \
   mulle_objc_metaabi_param_block( void *, return_type)

// quite the same, as we can't define void member. So just ignore
#define mulle_objc_metaabi_param_block_void_return( param_type) \
   mulle_objc_metaabi_param_block( param_type, void *)

#define mulle_objc_metaabi_param_block_void_parameter( return_type) \
   mulle_objc_metaabi_param_block( void *, return_type)

#endif
