//
//  mulle_objc_html.h
//  mulle-objc-runtime
//
//  Created by Nat! on 10.05.16.
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
#ifndef mulle_objc_html_h__
#define mulle_objc_html_h__

#include <stdarg.h>
#include <stdint.h>


struct _mulle_objc_universe;
struct _mulle_objc_staticstring;
struct _mulle_objc_class;
struct _mulle_objc_fastclasstable;
struct _mulle_objc_infraclass;
struct _mulle_objc_ivarlist;
struct _mulle_objc_propertylist;
struct _mulle_objc_cache;
struct _mulle_objc_methodlist;
struct _mulle_objc_descriptor;
struct _mulle_objc_uniqueidarray;

struct mulle_concurrent_pointerarray;
struct mulle_concurrent_hashmap;

struct _mulle_objc_htmltablestyle
{
   char  *title;
   char  *classprefix;  // no class prefix means no css emission
   char  *color;        // used if classprefix is not set
   char  *bgcolor;      // used if classprefix is not set
   int   colspan;       // optional
};

char   *mulle_objc_universe_describe_html( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_staticstring_describe_html( struct _mulle_objc_staticstring *string,
                                                struct _mulle_objc_htmltablestyle *styling);
char  *mulle_objc_staticstring_describe_hor_html( struct _mulle_objc_staticstring *string,
                                                  struct _mulle_objc_htmltablestyle *styling);
char  *mulle_objc_staticstring_describe_row_html( void *value,
                                                     struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_infraclass_describe_row_html( intptr_t  classid,
                                                    void *cls,
                                                    struct _mulle_objc_htmltablestyle *styling);
char  *mulle_objc_fastclassentry_describe_row_html( unsigned int i,
                                                       struct _mulle_objc_infraclass *infra,
                                                       struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_classestoload_describe_row( intptr_t classid,
                                                 void *cls,
                                                 struct _mulle_objc_htmltablestyle *styling);
char  *mulle_objc_categoriestoload_describe_row( intptr_t classid,
                                                    void *cls,
                                                    struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_class_describe_html( struct _mulle_objc_class *cls,
                                          struct _mulle_objc_htmltablestyle *styling);
char  *mulle_objc_class_describe_html_short( struct _mulle_objc_class *cls,
                                                struct _mulle_objc_htmltablestyle *styling);
char   *mulle_objc_class_describe_html_tiny( struct _mulle_objc_class *cls,
                                               struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_class_describe_row_html( intptr_t classid,
                                              void *cls,
                                              struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_ivarlist_describe_html( struct _mulle_objc_ivarlist *list,
                                              struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_ivarlist_describe_hor_html( struct _mulle_objc_ivarlist *list,
                                                 struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_propertylist_describe_html( struct _mulle_objc_propertylist *list,
                                                 struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_cache_describe_html( struct _mulle_objc_cache *cache,
                                           struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_descriptor_describe_html( struct _mulle_objc_descriptor *desc,
                                                struct _mulle_objc_htmltablestyle *styling);
char  *mulle_objc_descriptor_describe_hor_html( struct _mulle_objc_descriptor *desc);
char  *mulle_objc_descriptor_describe_row_html( intptr_t  methodid,
                                                         void *descriptor,
                                                         struct _mulle_objc_htmltablestyle *styling);
char   *mulle_objc_category_describe_row_html( intptr_t  categoryid,
                                                  void *value,
                                                  struct _mulle_objc_htmltablestyle *styling);
char   *mulle_objc_protocol_describe_row_html( intptr_t  methodid,
                                                  void *value,
                                                  struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_super_describe_row_html( intptr_t  superid,
                                               void *value,
                                               struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_methodlist_describe_html( struct _mulle_objc_methodlist *list,
                                                struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_htmltablestyle *styling);

char  *mulle_objc_methodlist_describe_hor_html( struct _mulle_objc_methodlist *list,
                                                   struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_protocols_describe_html( struct _mulle_objc_uniqueidarray *array,
                                               struct _mulle_objc_universe *universe,
                                               struct _mulle_objc_htmltablestyle *styling);
char   *mulle_objc_categories_describe_html( struct _mulle_objc_uniqueidarray *array,
                                                struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_loadclass_describe_row_html( void *value,
                                                   struct _mulle_objc_htmltablestyle *styling);
char   *mulle_objc_loadcategory_describe_row_html( void *value,
                                                      struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_fastclasstable_describe_html( struct _mulle_objc_fastclasstable *fastclasstable,
                                                     char *(row_description)( unsigned int i,
                                                        struct _mulle_objc_infraclass *,
                                                        struct _mulle_objc_htmltablestyle *),
                                                     struct _mulle_objc_htmltablestyle *styling);

char   *mulle_concurrent_pointerarray_describe_html( struct   mulle_concurrent_pointerarray *list,
                                                        char *(row_description)( void *,                struct _mulle_objc_htmltablestyle *),
                                                        struct _mulle_objc_htmltablestyle *styling);
char   *mulle_concurrent_hashmap_describe_html( struct mulle_concurrent_hashmap *map,
                                                   char *(row_description)( intptr_t, void *, struct _mulle_objc_htmltablestyle *),
                                                   struct _mulle_objc_htmltablestyle *styling);

char   *mulle_objc_uniqueidarray_describe_html( struct _mulle_objc_uniqueidarray *array,
                                                   char *(row_description)( void *, struct _mulle_objc_universe *, struct _mulle_objc_htmltablestyle *),
                                                   struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_htmltablestyle *styling);


int   mulle_objc_vasprintf( char **output, char *format, va_list args);
int   mulle_objc_asprintf( char **output, char *format, ...);


#endif /* mulle_objc_html_h */
