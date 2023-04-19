//
//  mulle_objc_csvdump.h
//  mulle-objc-debug-universe
//
//  Created by Nat! on 16.05.17.
//  Copyright © 2017 Mulle kybernetiK. All rights reserved.
//  Copyright © 2017 Codeon GmbH. All rights reserved.
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
#ifndef mulle_objc_csvdump_h__
#define mulle_objc_csvdump_h__

#include "include.h"

#include <stdio.h>

struct _mulle_objc_class;
struct _mulle_objc_universe;
struct _mulle_objc_loadinfo;


MULLE_OBJC_RUNTIME_GLOBAL
char   *_mulle_objc_get_tmpdir( void);


MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_classcoverage_to_fp( struct _mulle_objc_universe *universe,
                                                        FILE *fp);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_methodcoverage_to_fp( struct _mulle_objc_universe *universe,
                                                         FILE *fp);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_cachedmethodcoverage_to_fp( struct _mulle_objc_universe *universe,
                                                               FILE *fp);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_cachesizes_to_fp( struct _mulle_objc_universe *universe,
                                                     FILE *fp);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_class_csvdump_methodcoverage_to_fp( struct _mulle_objc_class *cls,
                                                      FILE *fp);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_class_csvdump_cachedmethodcoverage_to_fp( struct _mulle_objc_class *cls,
                                                            FILE *fp);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_class_csvdump_cachesizes_to_fp( struct _mulle_objc_class *cls,
                                                  FILE *fp);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_loadinfo_csvdump_terse_to_fp( struct _mulle_objc_loadinfo *info, FILE *fp);

// appends to existing files
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_methodcoverage_to_filename( struct _mulle_objc_universe *universe,
                                                               char *filename);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_classcoverage_to_filename( struct _mulle_objc_universe *universe,
                                                              char *filename);
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_universe_csvdump_cachesizes_to_filename( struct _mulle_objc_universe *universe,
                                                           char *filename);

#endif
