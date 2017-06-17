//
//  mulle_objc_dotdump.h
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

#ifndef mulle_objc_dotdump_h__
#define mulle_objc_dotdump_h__

#include <stdio.h>

struct _mulle_objc_class;
struct _mulle_objc_classpair;
struct _mulle_objc_universe;
struct _mulle_objc_methodlist;

//
// mulle_objc_dotdump_to_tmp is what you want: it dumps the universe and all
// classes and the classes are linked from the universe
// .dot file
//

#pragma mark - dump to /tmp (simpler)

void   mulle_objc_dotdump_to_tmp( void);

void   mulle_objc_dotdump_universe_to_tmp( void);
void   mulle_objc_dotdump_overview_to_tmp( void);
void   mulle_objc_class_dotdump_to_tmp( struct _mulle_objc_class *cls);
void   mulle_objc_dotdump_classname_to_tmp( char *classname);
void   mulle_objc_dotdump_classes_to_tmp( void);


#pragma mark - dump to working directory (safer)

void   mulle_objc_dotdump( void);

void   mulle_objc_dotdump_overview( void);
void   mulle_objc_dotdump_universe( void);
void   mulle_objc_class_dotdump( struct _mulle_objc_class *cls);
void   mulle_objc_dotdump_classname( char *classname);
void   mulle_objc_dotdump_classes( void);


#pragma mark - "movie" support

void   mulle_objc_dotdump_universe_frame_to_tmp( void);


#pragma mark - stuff for the debugger

void   mulle_objc_methodlist_dump( struct _mulle_objc_methodlist *list);


#endif
