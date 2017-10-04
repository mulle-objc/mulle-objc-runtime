//
//  common.inc
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
#include "mulle_objc_html.h"

#include "mulle_objc_runtime.h"
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>


#pragma mark - printing support
// lazy copout
static char   *html_escape( char *s)
{
   if( ! strchr( s, '&') && ! strchr( s, '<'))
      return( s);

   return( "bad-html");
}

//
// have my own, because asprintf is a gnu extension, whereas
// vsnprintf should be C99. Use malloc because that's what vsprintf does
//
int   mulle_objc_vasprintf( char **output, char *format, va_list args)
{
   int      size;
   va_list  tmp;

   va_copy( tmp, args);
   size = vsnprintf( NULL, 0, format, tmp);
   va_end( tmp);

   if( size < 0)
      return( size);

   *output = (char *) malloc( size + 1);
   if( ! *output)
      return( -1);

   return( vsprintf( *output, format, args));
}


int   mulle_objc_asprintf( char **output, char *format, ...)
{
   va_list  args;
   int      size;

   va_start( args, format);
   size = mulle_objc_vasprintf( output, format, args);
   va_end(args);

   return size;
}


#define asprintf  mulle_objc_asprintf



//
// stuff shared by graphviz and html, don't want to expose the
// symbols or dick around with linker options
//

static char  *inheritance_description( unsigned int inheritance)
{
   char     *tmp;
   size_t   len;

   tmp = malloc( 128);
   if( ! tmp)
      return( tmp);

   tmp[ 0] = 0;
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
      strcat( tmp, "superclass ");
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES))
      strcat( tmp, "categories ");
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
      strcat( tmp, "protocols ");
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES))
      strcat( tmp, "protocol_categories ");

   len = strlen( tmp);
   if( len)
      tmp[ len - 1] = 0;
   return( tmp);
}


static char  *categoryid_html_row_description( void *value,
                                               struct _mulle_objc_universe *universe,
                                               struct _mulle_objc_htmltablestyle *styling)
{
   mulle_objc_categoryid_t   categoryid;
   char                      *s;
   char                      *result;

   categoryid = (mulle_objc_categoryid_t) (intptr_t) value;
   s          = _mulle_objc_universe_string_for_categoryid( universe,
                                                           categoryid);

   mulle_objc_asprintf( &result, "<TR><TD>\"%s\"</TD><TD>%08x</TD></TR>\n",
         s, categoryid);
   return( result);
}


static char  *protocolid_html_row_description( void *value,
                                               struct _mulle_objc_universe *universe,
                                               struct _mulle_objc_htmltablestyle *styling)
{
   mulle_objc_protocolid_t   protocolid;
   char                      *s;
   char                      *result;

   protocolid = (mulle_objc_protocolid_t) (intptr_t) value;
   s          = _mulle_objc_universe_string_for_protocolid( universe, protocolid);

   mulle_objc_asprintf( &result, "<TR><TD>\"%s\"</TD><TD>%08x</TD></TR>\n",
         s, protocolid);
   return( result);
}


#pragma mark - universe

static void   asprintf_table_header_colspan( char **s,
                                             struct _mulle_objc_htmltablestyle *styling,
                                             unsigned int colspan)
{
   if( styling->classprefix)
      asprintf( s,
               "<TABLE CLASS=\"%s_table\">\n<TR CLASS=\"%s_table_header\"><TH COLSPAN=\"%u\">%s</TH></TR>\n",
               styling->classprefix,
               styling->classprefix,
               colspan ? colspan : 2,
               styling->title);
   else // graphviz don't like CLASS, can't deal with TH
      asprintf( s,
               "<TABLE>\n<TR><TD BGCOLOR=\"%s\" COLSPAN=\"%u\"><FONT COLOR=\"%s\">%s</FONT></TD></TR>\n",
               styling->bgcolor,
               colspan ? colspan : 2,
               styling->color,
               styling->title);
}


static void   asprintf_table_header( char **s, struct _mulle_objc_htmltablestyle *styling)
{
   asprintf_table_header_colspan( s, styling, styling->colspan);
}


static char  *final_concat_malloced_tmp_known_len( char **tmp, unsigned int n, size_t len)
{
   unsigned int  i;
   char          *s;

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}


static char  *final_concat_auto_tmp_known_len( char **tmp, unsigned int n, size_t len)
{
   unsigned int  i;
   char          *s;

   s = malloc( len + 1);
   s[ 0] = 0;

   for( i = 0; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   return( s);
}


static char  *final_concat_auto_tmp( char **tmp, unsigned int n)
{
   unsigned int  i;
   size_t        len;

   len = 0;
   for( i = 0; i < n; i++)
      len += strlen( tmp[ i]);

   return( final_concat_auto_tmp_known_len( tmp, n, len));
}


char   *mulle_objc_universe_html_description( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_htmltablestyle *styling)
{
   char           *tmp[ 3];

   // create single lines for each method and two for head/tail
   asprintf_table_header( &tmp[ 0], styling);

   asprintf( &tmp[ 1],
            "<TR><TD>version</TD><TD>0x%x</TD></TR>\n",
            universe->version);
   asprintf( &tmp[ 2],
            "</TABLE>");

   return( final_concat_auto_tmp( tmp, 3));
}



#pragma mark - static strings


char  *mulle_objc_staticstring_html_description( struct _mulle_objc_staticstring *string,
                                                 struct _mulle_objc_htmltablestyle *styling)
{
   char   *tmp[ 4];
   char   *th;

   th = styling->classprefix ? "TH" : "TD";
   // create single lines for each method and two for head/tail
   asprintf_table_header( &tmp[ 0], styling);

   asprintf( &tmp[ 1],
            "<TR><%s>_s</%s><TD>%s</TD></TR>\n",
               th, th,
               html_escape( string->_s ? string->_s : "*null*"));
   asprintf( &tmp[ 2],
            "<TR><%s>_len</%s><TD>%d</TD></TR>\n",
               th, th,
               string->_len);
   asprintf( &tmp[ 3],
            "</TABLE>");

   return( final_concat_auto_tmp( tmp, 4));
}


char  *mulle_objc_staticstring_hor_html_description( struct _mulle_objc_staticstring *string,
                                                     struct _mulle_objc_htmltablestyle *styling)
{
   char *s;

   asprintf( &s, "%s", html_escape( string->_s ? string->_s : "*null*"));

   return( s);
}


char  *mulle_objc_staticstring_html_row_description( void *value,
                                                     struct _mulle_objc_htmltablestyle *styling)
{
   struct _mulle_objc_staticstring *string = value;
   char   *s;

   asprintf( &s,
            "<TR>"
            "<TD COLSPAN=\"2\">%s</TD>"
            "</TR>\n",
            html_escape( string->_s ? string->_s : "*null*"));

   return( s);
}


char  *mulle_objc_fastclassentry_html_row_description( unsigned int i,
                                                       struct _mulle_objc_infraclass *infra,
                                                       struct _mulle_objc_htmltablestyle *styling)
{
   char   *s;

   asprintf( &s,
            "<TR>"
            "<TD>%u</TD>"
            "<TD>%s</TD>"
            "</TR>\n",
            i,
            html_escape( infra ? _mulle_objc_infraclass_get_name( infra) : "*null*"));

   return( s);
}

#pragma mark - classes

char   *mulle_objc_class_short_html_description( struct _mulle_objc_class *cls,
                                                 struct _mulle_objc_htmltablestyle *styling)
{
   char  *s;
   char  *name;

   name   = html_escape( cls->name);
   asprintf( &s, "<a href=\"%s.html\">%s</a>\n",
            name,
            name);
   return( s);
}


char   *mulle_objc_class_html_row_description( intptr_t  classid,
                                               void *cls,
                                               struct _mulle_objc_htmltablestyle *styling)
{
   char   *s;
   char   *name;
   char   *prefix;

   name   = html_escape( _mulle_objc_class_get_name( cls));
   prefix = _mulle_objc_class_is_metaclass( cls) ? "+" : "";

   asprintf( &s, "<TR><TD><A HREF=\"%s%s.html\">%s</A></TD><TD>%08x</TD></TR>\n",
            prefix,
            name,
            name,
            _mulle_objc_class_get_classid( cls));
   return( s);
}


char   *mulle_objc_infraclass_html_row_description( intptr_t  classid,
                                                    void *cls,
                                                    struct _mulle_objc_htmltablestyle *styling)
{
   char   *s;
   char   *name;

   name   = html_escape( _mulle_objc_infraclass_get_name( cls));
   asprintf( &s, "<TR><TD>%s</TD><TD>%08x</TD></TR>\n",
            name,
            _mulle_objc_infraclass_get_classid( cls));
   return( s);
}


char   *mulle_objc_class_html_tiny_description( struct _mulle_objc_class *cls,
                                                struct _mulle_objc_htmltablestyle *styling)
{
   char          *tmp[ 2];
   unsigned int  i;

   i = 0;
   asprintf_table_header( &tmp[ i++], styling);
   asprintf( &tmp[ i++], "</TABLE>");

   assert( i <= 2);

   return( final_concat_auto_tmp( tmp, i));
}


char   *mulle_objc_class_html_description( struct _mulle_objc_class *cls,
                                           struct _mulle_objc_htmltablestyle *styling)
{
   char           *s;
   char           *tmp[ 7];
   unsigned int   i;
   struct _mulle_objc_htmltablestyle  style;

   style       = *styling;
   style.title = html_escape( cls->name);

   i = 0;

   // create single lines for each method and two for head/tail
   // fummel for graphviz
   asprintf_table_header( &tmp[ i++], &style);

   asprintf( &tmp[ i++],
            "<TR><TD>allocationsize</TD><TD>%lu</TD></TR>\n",
            cls->allocationsize);

   s = inheritance_description( _mulle_objc_class_get_inheritance( cls));
   asprintf( &tmp[ i++],
            "<TR><TD>inheritance</TD><TD>%s</TD></TR>\n",
            s);

   free( s);

   asprintf( &tmp[ i++],
            "<TR><TD>state</TD><TD>0x%lx</TD></TR>\n",
            (long) _mulle_atomic_pointer_nonatomic_read( &cls->state));

   if ( _mulle_objc_class_is_infraclass( cls))
   {
      struct _mulle_objc_infraclass   *infra;

      infra = _mulle_objc_class_as_infraclass( cls);
      asprintf( &tmp[ i++],
               "<TR><TD>ivarhash</TD><TD>0x%lx</TD></TR>\n",
               (long) infra->ivarhash);
   }

   asprintf( &tmp[ i++],
            "<TR><TD>preloads</TD><TD>0x%x</TD></TR>\n",
            cls->preloads);

   asprintf( &tmp[ i++],
            "</TABLE>");

   assert( i <= 7);

   return( final_concat_auto_tmp( tmp, i));
}


#pragma mark - ivarlist

char   *mulle_objc_ivarlist_html_description( struct _mulle_objc_ivarlist *list,
                                              struct _mulle_objc_htmltablestyle *styling)
{
   size_t         len;
   char           **tmp;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;

   n    = list->n_ivars + 2;
   tmp  = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   // create single lines for each method and two for head/tail
   i = 0;
   asprintf_table_header( &tmp[ i], styling);
   len = strlen( tmp[ i]);
   ++i;

   for( j = 0; j < list->n_ivars; j++)
   {
      asprintf( &tmp[ i], "<TR>"
               "<TD>%s</TD>"
               "<TD>"
                 "<TABLE>"
                   "<TR><TD>signature</TD><TD>%s</TD></TR>"
                   "<TR><TD>methodid</TD><TD>0x%lx</TD></TR>"
                   "<TR><TD>offset</TD><TD>%d</TD></TR>"
                 "</TABLE>"
               "</TD>"
               "</TR>\n",
               html_escape( list->ivars[ j].descriptor.name),
               html_escape( list->ivars[ j].descriptor.signature),
               (long) list->ivars[ j].descriptor.ivarid,
               list->ivars[ j].offset);

      len += strlen( tmp[ i]);
      ++i;
   }

   asprintf( &tmp[ i], "</TABLE>");
   len += strlen( tmp[ i]);
   ++i;

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


char   *mulle_objc_ivarlist_html_hor_description( struct _mulle_objc_ivarlist *list,
                                                  struct _mulle_objc_htmltablestyle *styling)
{
   size_t         len;
   char           **tmp;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;

   n   = list->n_ivars + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   // create single lines for each method and two for head/tail
   i = 0;
   asprintf_table_header_colspan( &tmp[ i], styling, 4);
   len = strlen( tmp[ i]);
   ++i;

   for( j = 0; j < list->n_ivars; j++)
   {
      asprintf( &tmp[ i],
               "<TR>"
                 "<TD>%s</TD>"
                 "<TD>%s</TD>"
                 "<TD>%08x</TD>"
                 "<TD>%d</TD>"
               "</TR>\n",
               html_escape( list->ivars[ j].descriptor.name),
               html_escape( list->ivars[ j].descriptor.signature),
               list->ivars[ j].descriptor.ivarid,
               list->ivars[ j].offset);
      len += strlen( tmp[ i]);
      ++i;
   }

   asprintf( &tmp[ i], "</TABLE>");
   len += strlen( tmp[ i]);
   ++i;

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - descriptor


char   *mulle_objc_descriptor_html_description( struct _mulle_objc_descriptor *desc,
                                                struct _mulle_objc_htmltablestyle *styling)
{
   char   *tmp[ 2];
   char   *th;

   th = styling->classprefix ? "TH" : "TD";

   asprintf_table_header( &tmp[ 0], styling);
   asprintf( &tmp[ 1],
            "<TR><%s>name</%s><TD>%s</TD></TR>"
            "<TR><%s>signature</%s><TD>%s</TD></TR>"
            "<TR><%s>methodid</%s><TD>%08x</TD></TR>"
            "<TR><%s>bits</%s><TD>0x%x</TD></TR>"
            "</TABLE>",
            th, th,
            html_escape( desc->name),
            th, th,
            html_escape( desc->signature),
            th, th,
            desc->methodid,
            th, th,
            desc->bits);

   return( final_concat_auto_tmp( tmp, 2));
}


char   *mulle_objc_descriptor_html_hor_description( struct _mulle_objc_descriptor *desc)
{
   char   *s;

   asprintf( &s,
            "<TR>"
            "<TD>%s</TD>"
            "<TD>%s</TD>"
            "<TD>%08x</TD>"
            "<TD>0x%x</TD>"
            "</TR>",
            html_escape( desc->name),
            html_escape( desc->signature),
            desc->methodid,
            desc->bits);

   return( s);
}


char   *mulle_objc_descriptor_html_row_description( intptr_t  methodid,
                                                    void *value,
                                                    struct _mulle_objc_htmltablestyle *styling)
{
   struct _mulle_objc_descriptor *desc = value;
   char   *s;

   asprintf( &s,
            "<TR>"
            "<TD>%s</TD>"
            "<TD>%s</TD>"
            "<TD>%08x</TD>"
            "<TD>0x%x</TD>"
            "</TR>\n",
            html_escape( desc->name),
            html_escape( desc->signature),
            desc->methodid,
            desc->bits);

   return( s);
}


#pragma mark - categories

char   *mulle_objc_category_html_row_description( intptr_t  categoryid,
                                                  void *value,
                                                  struct _mulle_objc_htmltablestyle *styling)
{
   char  *s;

   asprintf( &s,
            "<TR>"
            "<TD>%s</TD>"
            "<TD>%08x</TD>"
            "</TR>\n",
            html_escape( value),
            categoryid);

   return( s);
}

#pragma mark - supers

char   *mulle_objc_super_html_row_description( intptr_t  superid,
                                               void *value,
                                               struct _mulle_objc_htmltablestyle *styling)
{
   struct _mulle_objc_super  *superinfo = value;
   char  *s;
   
   asprintf( &s,
            "<TR>"
            "<TD>%s</TD>"
            "<TD>%08x</TD>"
            "</TR>\n",
            html_escape( _mulle_objc_super_get_name( superinfo)),
            superid);
   
   return( s);
}


#pragma mark - protocols

char   *mulle_objc_protocol_html_row_description( intptr_t  protocolid,
                                                  void *value,
                                                  struct _mulle_objc_htmltablestyle *styling)
{
   struct _mulle_objc_protocol  *protocol = value;
   char  *s;

   asprintf( &s,
            "<TR>"
            "<TD>%s</TD>"
            "<TD>%08x</TD>"
            "</TR>\n",
            html_escape( _mulle_objc_protocol_get_name( protocol)),
            protocolid);

   return( s);
}



#pragma mark - propertylist

char   *mulle_objc_propertylist_html_description( struct _mulle_objc_propertylist *list,
                                                  struct _mulle_objc_htmltablestyle *styling)
{
   size_t         len;
   char           **tmp;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;

   n   = list->n_properties + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   // create single lines for each method and two for head/tail
   i = 0;
   asprintf_table_header( &tmp[ i], styling);
   len = strlen( tmp[ i]);
   ++i;

   for( j = 0; j < list->n_properties; j++)
   {
      asprintf( &tmp[ i],
               "<TR><TD>%s</TD>"
               "<TD>"
                 "<TABLE>"
                   "<TR><TD>signature</TD><TD>%s</TD></TR>"
                   "<TR><TD>propertyid</TD><TD>%08x</TD></TR>"
                   "<TR><TD>ivarid</TD><TD>%08x</TD></TR>"
                   "<TR><TD>getter</TD><TD>%08x</TD></TR>"
                   "<TR><TD>setter</TD><TD>%08x</TD></TR>"
                   "<TR><TD>clearer</TD><TD>%08x</TD></TR>"
                 "</TABLE>"
               "</TD>"
               "</TR>\n",
               html_escape( list->properties[ j].name),
               html_escape( list->properties[ j].signature),
               list->properties[ j].propertyid,
               list->properties[ j].ivarid,
               list->properties[ j].getter,
               list->properties[ j].setter,
               list->properties[ j].clearer);

      len += strlen( tmp[ i]);
      ++i;
   }

   asprintf( &tmp[ i], "</TABLE>");
   len += strlen( tmp[ i]);
   ++i;

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - cache

char   *mulle_objc_cache_html_description( struct _mulle_objc_cache *cache,
                                           struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_htmltablestyle *styling)
{
   size_t          len;
   char            **tmp;
   unsigned int    i;
   unsigned int    j;
   unsigned int    n;
   int             index;
   mulle_objc_methodid_t   sel;
   
   n   = cache->size + 3 + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   i = 0;
   asprintf_table_header_colspan( &tmp[ i], styling, 5);
   len = strlen( tmp[ i]);
   ++i;

   asprintf( &tmp[ i],
            "<TR><TD>n</TD><TD COLSPAN=\"4\">%lu</TD></TR>\n",
            (long) _mulle_atomic_pointer_nonatomic_read( &cache->n));
   len += strlen( tmp[ i]);
   ++i;
   asprintf( &tmp[ i],
            "<TR><TD>mask</TD><TD COLSPAN=\"4\">0x%lx</TD></TR>\n",
             (long) cache->mask);
   len += strlen( tmp[ i]);
   ++i;


   for( j = 0; j < cache->size; j++)
   {
      index = 0;
      sel   = cache->entries[ j].key.uniqueid;
      if( sel)
         index = _mulle_objc_cache_find_entryindex( cache, sel);

      asprintf( &tmp[ i],
               "<TR><TD>#%ld</TD><TD>%08x</TD><TD>%s</TD><TD>%p</TD><TD>%d (%x)</TD></TR>\n",
               j,
               sel,
               _mulle_objc_universe_string_for_methodid( universe, sel),
               cache->entries[ j].value.functionpointer,
               index,
               sel & cache->mask);

      len += strlen( tmp[ i]);
      ++i;
   }

   asprintf( &tmp[ i], "</TABLE>");
   len += strlen( tmp[ i]);
   ++i;

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - methodlist

char   *mulle_objc_methodlist_html_description( struct _mulle_objc_methodlist *list,
                                                struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_htmltablestyle *styling)
{
   size_t         len;
   char           **tmp;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;
   char           buf[ s_mulle_objc_sprintf_functionpointer_buffer];
   
   n   = list->n_methods + 3;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   // create single lines for each method and two for head/tail
   i = 0;
   asprintf_table_header( &tmp[ i], styling);
   len = strlen( tmp[ i]);
   ++i;

   
   if( universe)
   {
      asprintf( &tmp[ i], "<TR><TD>owner</TD><TD>%s</TD></TR>",
                           _mulle_objc_universe_string_for_categoryid( universe,
                                                                      (mulle_objc_categoryid_t) (uintptr_t) list->owner));
   }
   else
   {
      asprintf( &tmp[ i], "<TR><TD>owner</TD><TD>%p</TD></TR>", list->owner);
   }
   len += strlen( tmp[ i]);
   ++i;
   
   for( j = 0; j < list->n_methods; j++)
   {
      mulle_objc_sprintf_functionpointer( buf,
         _mulle_atomic_functionpointer_nonatomic_read( &list->methods[ j].implementation));

      asprintf( &tmp[ i],
               "<TR>"
               "<TD>%s</TD>"
               "<TD>"
                 "<TABLE>"
                   "<TR><TD>signature</TD><TD>%s</TD></TR>"
                   "<TR><TD>methodid</TD><TD>0x%lx</TD></TR>"
                   "<TR><TD>bits</TD><TD>0x%x</TD></TR>"
                   "<TR><TD>implementation</TD><TD>%s</TD></TR>"
                 "</TABLE>"
               "</TD>"
               "</TR>\n",
               html_escape( list->methods[ j].descriptor.name),
               html_escape( list->methods[ j].descriptor.signature),
               (long) list->methods[ j].descriptor.methodid,
               list->methods[ j].descriptor.bits,
               buf);

      len += strlen( tmp[ i]);
      ++i;
   }

   asprintf( &tmp[ i], "</TABLE>");
   len += strlen( tmp[ i]);
   ++i;

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


char   *mulle_objc_methodlist_html_hor_description( struct _mulle_objc_methodlist *list,
                                                    struct _mulle_objc_htmltablestyle *styling)
{
   size_t         len;
   char           **tmp;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;
   char           buf[ s_mulle_objc_sprintf_functionpointer_buffer];

   n   = list->n_methods + 3;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   // create single lines for each method and two for head/tail
   i = 0;
   asprintf_table_header_colspan( &tmp[ i], styling, 5);
   len = strlen( tmp[ i]);
   ++i;

   if( list->n_methods)
   {
      if( styling->classprefix)
         asprintf( &tmp[ i],
                   "<TR>"
                     "<TH>name</TH>"
                     "<TH>signature</TH>"
                     "<TH>methodid</TH>"
                     "<TH>bits</TH>"
                     "<TH>implementation</TH>"
                   "</TR>\n");
      else
         asprintf( &tmp[ i],
                   "<TR>"
                     "<TD>name</TD>"
                     "<TD>signature</TD>"
                     "<TD>methodid</TD>"
                     "<TD>bits</TD>"
                     "<TD>implementation</TD>"
                   "</TR>\n");

      len += strlen( tmp[ i]);
      ++i;
   }

   for( j = 0; j < list->n_methods; j++)
   {
      mulle_objc_sprintf_functionpointer( buf,
         _mulle_atomic_functionpointer_nonatomic_read( &list->methods[ j].implementation));

      asprintf( &tmp[ i],
               "<TR>"
                 "<TD>%s</TD>"
                 "<TD>%s</TD>"
                 "<TD>%08x</TD>"
                 "<TD>0x%x</TD>"
                 "<TD>%s</TD>"
               "</TR>\n",
               html_escape( list->methods[ j].descriptor.name),
               html_escape( list->methods[ j].descriptor.signature),
               list->methods[ j].descriptor.methodid,
               list->methods[ j].descriptor.bits,
               buf);
      len += strlen( tmp[ i]);
      ++i;
   }

   asprintf( &tmp[ i], "</TABLE>");
   len += strlen( tmp[ i]);
   ++i;

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - loadclass

char   *mulle_objc_loadclass_html_row_description( void *value,
                                                   struct _mulle_objc_htmltablestyle *styling)
{
   struct _mulle_objc_loadclass   *loadcls = value;
   char   *s;

   asprintf( &s, "<TR><TD>%s</TD><TD>%08x</TD></TR>\n",
            loadcls->classname,
            loadcls->classid);
   return( s);
}


#pragma mark - loadcategory

char   *mulle_objc_loadcategory_html_row_description( void *value,
                                                      struct _mulle_objc_htmltablestyle *styling)
{
   struct _mulle_objc_loadcategory   *loadcat = value;
   char   *s;

   asprintf( &s, "<TR><TD>%s( %s)</TD><TD>%08x</TD></TR>\n",
            loadcat->classname,
            loadcat->categoryname,
            loadcat->categoryid);
   return( s);
}



#pragma mark - protocols

char   *mulle_objc_protocols_html_description( struct _mulle_objc_uniqueidarray *array,
                                               struct _mulle_objc_universe *universe,
                                               struct _mulle_objc_htmltablestyle *styling)
{
   return( mulle_objc_uniqueidarray_html_description( array,
                                                      protocolid_html_row_description,
                                                      universe,
                                                      styling));
}


#pragma mark - categories

char   *mulle_objc_categories_html_description( struct _mulle_objc_uniqueidarray *array,
                                                struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_htmltablestyle *styling)
{
   return( mulle_objc_uniqueidarray_html_description( array,
                                                      categoryid_html_row_description,
                                                      universe,
                                                      styling));
}


#pragma mark - fastclasses

char   *mulle_objc_fastclasstable_html_description( struct _mulle_objc_fastclasstable *fastclasstable,
                                                     char *(row_description)( unsigned int i,
                                                        struct _mulle_objc_infraclass *,
                                                        struct _mulle_objc_htmltablestyle *),
                                                     struct _mulle_objc_htmltablestyle *styling)

{
   size_t         count;
   size_t         len;
   char           **tmp;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;
   void           *value;

   count = MULLE_OBJC_S_FASTCLASSES;

   n   = (unsigned int) count + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   tmp[ 0] = NULL;

   i   = 0;
   len = 0;

   if( styling)
   {
      asprintf_table_header( &tmp[ i], styling);
      len += strlen( tmp[ i]);
      ++i;
   }

   for( j = 0; j < MULLE_OBJC_S_FASTCLASSES; j++)
   {
      value = _mulle_atomic_pointer_nonatomic_read( &fastclasstable->classes[ j].pointer);
      tmp[ i] = (*row_description)( j, value, styling);
      len    += strlen( tmp[ i]);
      ++i;
   }

   if( styling)
   {
      asprintf( &tmp[ i], "</TABLE>");
      len += strlen( tmp[ i]);
      ++i;
   }

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - pointerarray

char   *mulle_concurrent_pointerarray_html_description( struct mulle_concurrent_pointerarray *list,
                                                        char *(row_description)( void *, struct _mulle_objc_htmltablestyle *),
                                                        struct _mulle_objc_htmltablestyle *styling)

{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   size_t                                           count;
   size_t                                           len;
   char                                             **tmp;
   unsigned int                                     i;
   unsigned int                                     n;
   void                                             *value;
   char                                             *null_description;

   count = mulle_concurrent_pointerarray_get_count( list);

   n   = (unsigned int) count + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   tmp[ 0] = NULL;

   i   = 0;
   len = 0;

   if( styling)
   {
      asprintf_table_header( &tmp[ i], styling);
      len += strlen( tmp[ i]);
      ++i;
   }

   null_description = "*null*";

   rover = mulle_concurrent_pointerarray_enumerate( list);
   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( ! value)
         value = null_description;
      tmp[ i] = (*row_description)( value, styling);
      len    += strlen( tmp[ i]);
      ++i;
   }

   if( styling)
   {
      asprintf( &tmp[ i], "</TABLE>");
      len += strlen( tmp[ i]);
      ++i;
   }

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - hashmap

char   *mulle_concurrent_hashmap_html_description( struct mulle_concurrent_hashmap *map,
                                                   char *(row_description)( intptr_t, void *, struct _mulle_objc_htmltablestyle *),
                                                   struct _mulle_objc_htmltablestyle *styling)
{
   struct mulle_concurrent_hashmapenumerator   rover;
   char                                        *null_description;
   intptr_t                                    uniqueid;
   size_t                                      count;
   size_t                                      len;
   char                                        **tmp;
   unsigned int                                i;
   unsigned int                                n;
   void                                        *value;

   count = mulle_concurrent_hashmap_count( map);

   n   = (unsigned int) count + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));


   i   = 0;
   len = 0;

   if( styling)
   {
      asprintf_table_header( &tmp[ i], styling);
      len += strlen( tmp[ i]);
      ++i;
   }

   null_description = "*null*";

   rover = mulle_concurrent_hashmap_enumerate( map);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &uniqueid, &value))
   {
      if( ! value)
         value = null_description;
      tmp[ i] = (*row_description)( uniqueid, value, styling);
      len    += strlen( tmp[ i]);
      ++i;
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   if( styling)
   {
      asprintf( &tmp[ i], "</TABLE>");
      len += strlen( tmp[ i]);
      ++i;
   }

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}


#pragma mark - uniqueidarray

char   *mulle_objc_uniqueidarray_html_description( struct _mulle_objc_uniqueidarray *array,
                                                   char *(row_description)( void *, struct _mulle_objc_universe *, struct _mulle_objc_htmltablestyle *),
                                                   struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_htmltablestyle *styling)

{
   unsigned int            count;
   size_t                  len;
   char                    **tmp;
   unsigned int            i;
   unsigned int            n;
   mulle_objc_uniqueid_t   *p;
   mulle_objc_uniqueid_t   *sentinel;
   
   count = array->n;

   n   = count + 2;
   tmp = mulle_allocator_calloc( &mulle_stdlib_allocator, n, sizeof( char *));

   tmp[ 0] = NULL;

   i   = 0;
   len = 0;

   if( styling)
   {
      asprintf_table_header( &tmp[ i], styling);
      len += strlen( tmp[ i]);
      ++i;
   }

   p        = array->entries;
   sentinel = &p[ count];
   
   while( p < sentinel)
   {
      tmp[ i] = (*row_description)( (void *) (uintptr_t) *p++, universe, styling);
      len    += strlen( tmp[ i]);
      ++i;
   }

   if( styling)
   {
      asprintf( &tmp[ i], "</TABLE>");
      len += strlen( tmp[ i]);
      ++i;
   }

   assert( i <= n);

   return( final_concat_malloced_tmp_known_len( tmp, i, len));
}

