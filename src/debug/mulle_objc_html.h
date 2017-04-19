//
//  mulle_objc_html.h
//  mulle-objc
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


struct _mulle_objc_runtime;
struct _mulle_objc_staticstring;
struct _mulle_objc_class;
struct _mulle_objc_ivarlist;
struct _mulle_objc_propertylist;
struct _mulle_objc_cache;
struct _mulle_objc_methodlist;
struct _mulle_objc_methoddescriptor;

struct mulle_concurrent_pointerarray;
struct mulle_concurrent_hashmap;

struct _mulle_objc_colored_string
{
   char  *text;
   char  *color;
   char  *backgroundColor;
};

char  *mulle_objc_runtime_html_description( struct _mulle_objc_runtime *runtime);

char  *mulle_objc_staticstring_html_description( struct _mulle_objc_staticstring *string);
char  *mulle_objc_staticstring_hor_html_description( struct _mulle_objc_staticstring *string);
char  *mulle_objc_staticstring_html_row_description( void *value);

char  *mulle_objc_classestoload_row_description( intptr_t classid, void *cls);
char  *mulle_objc_categoriestoload_row_description( intptr_t classid, void *cls);

char  *mulle_objc_class_html_description( struct _mulle_objc_class *cls, char *color);
char  *mulle_objc_class_short_html_description( struct _mulle_objc_class *cls);
char  *mulle_objc_class_html_row_description( intptr_t classid, void *cls);

char  *mulle_objc_ivarlist_html_description( struct _mulle_objc_ivarlist *list);
char  *mulle_objc_ivarlist_html_hor_description( struct _mulle_objc_ivarlist *list);

char  *mulle_objc_propertylist_html_description( struct _mulle_objc_propertylist *list);

char  *mulle_objc_cache_html_description( struct _mulle_objc_cache *cache);

char  *mulle_objc_methoddescriptor_html_description( intptr_t methodid, struct _mulle_objc_methoddescriptor *desc);
char  *mulle_objc_methoddescriptor_html_hor_description( struct _mulle_objc_methoddescriptor *desc);
char  *mulle_objc_methoddescriptor_html_row_description( intptr_t  methodid,
                                                         void *descriptor);

char  *mulle_objc_methodlist_html_description( struct _mulle_objc_methodlist *list);
char  *mulle_objc_methodlist_html_hor_description( struct _mulle_objc_methodlist *list);

char   *mulle_objc_protocols_html_description( struct   mulle_concurrent_pointerarray *list,
                                               struct _mulle_objc_colored_string *title);
char   *mulle_objc_categories_html_description( struct   mulle_concurrent_pointerarray *list,
                                                struct _mulle_objc_colored_string *title);

char   *mulle_objc_loadclass_html_row_description( void *value);
char   *mulle_objc_loadcategory_html_row_description( void *value);

char   *mulle_concurrent_pointerarray_html_description( struct   mulle_concurrent_pointerarray *list,
                                                       char *(row_description)( void *),
                                                       struct _mulle_objc_colored_string *title);
char   *mulle_concurrent_hashmap_html_description( struct mulle_concurrent_hashmap *map,
                                                  char *(row_description)( intptr_t, void *),
                                                  struct _mulle_objc_colored_string *title);


int   mulle_objc_vasprintf( char **output, char *format, va_list args);
int   mulle_objc_asprintf( char **output, char *format, ...);


#endif /* mulle_objc_html_h */
