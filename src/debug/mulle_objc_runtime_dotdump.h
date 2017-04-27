//
//  mulle_objc_runtime_dotdump.h
//  mulle-objc
//
//  Created by Nat! on 25.10.15.
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

//

#ifndef mulle_objc_runtime_dotdump_h__
#define mulle_objc_runtime_dotdump_h__

#include <stdio.h>

struct _mulle_objc_class;
struct _mulle_objc_classpair;
struct _mulle_objc_runtime;
struct _mulle_objc_methodlist;

//
// dumping to graphviz is nice, if you are dealing with the mulle-objc
// by itself. But soon it gets too complex for graphviz
//

void   _mulle_objc_runtime_dotdump( struct _mulle_objc_runtime *runtime, FILE *fp);

void   mulle_objc_dotdump_runtime( void);

void   mulle_objc_dotdump_runtime_to_file( char *filename);
void   mulle_objc_dotdump_runtime_to_tmp( void);


// dumps the class pairs, but pass in any class meta or infra
// for convenience

void   mulle_objc_classpair_dotdump( struct _mulle_objc_classpair *pair,
                                     FILE *fp);
void   mulle_objc_class_dotdump_to_file( struct _mulle_objc_class *cls,
                                         char *filename);
void   mulle_objc_dotdump_classname_to_file( char *classname,
                                             char *filename);
void   mulle_objc_dotdump_classname_to_tmp( char *classname);
void   mulle_objc_class_dotdump_to_tmp( struct _mulle_objc_class *cls);


# pragma mark - -
#pragma mark stuff for the debugger

void   mulle_objc_methodlist_dump( struct _mulle_objc_methodlist *list);

#endif
