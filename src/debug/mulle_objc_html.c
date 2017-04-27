//
//  common.inc
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
#include "mulle_objc_html.h"

#include "mulle_objc.h"
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>


static char   *html_escape( char *s)
{
   if( ! strchr( s, '&') && ! strchr( s, '<'))
      return( s);

   return( "*bad HTML*");
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


static char  *uniqueid_html_row_description( void *value)
{
   mulle_objc_uniqueid_t        uniqueid;
   char                         *s;
   char                         *result;
   
   uniqueid = (mulle_objc_uniqueid_t) (intptr_t) value;
   s        = mulle_objc_string_for_uniqueid( uniqueid);
   
   mulle_objc_asprintf( &result, "<TR><TD>%08x</TD><TD>\"%s\"</TD></TR>\n", uniqueid, s);
   return( result);
}


#pragma mark - runtime


char   *mulle_objc_runtime_html_description( struct _mulle_objc_runtime *runtime)
{
   size_t         len;
   char           *s;
   char           *tmp[ 3];
   unsigned int   i;

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0],
            "<TABLE>\n<TR><TD BGCOLOR=\"red\" COLSPAN=\"2\"><FONT COLOR=\"white\">runtime</FONT></TD></TR>\n");
   asprintf( &tmp[ 1],
            "<TR><TD>version</TD><TD>0x%x</TD></TR>\n",
               runtime->version);
   asprintf( &tmp[ 2],
            "</TABLE>");

   len = 0;
   for( i = 0; i < 3; i++)
      len += strlen( tmp[ i]);

   // concatenate all strings

   s = malloc( len + 1);
   s[ 0] = 0;

   for( i = 0; i < 3; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   return( s);
}



#pragma mark - static strings


char  *mulle_objc_staticstring_html_description( struct _mulle_objc_staticstring *string)
{
   size_t         len;
   char           *s;
   char           *tmp[ 4];
   unsigned int   i;

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0],
            "<TABLE>\n<TR><TD BGCOLOR=\"green\" COLSPAN=\"2\"><FONT COLOR=\"white\">string</FONT></TD></TR>\n");
   asprintf( &tmp[ 1],
            "<TR><TD>_s</TD><TD>%s</TD></TR>\n",
               html_escape( string->_s ? string->_s : "*null*"));
   asprintf( &tmp[ 2],
            "<TR><TD>_len</TD><TD>%d</TD></TR>\n",
               string->_len);
   asprintf( &tmp[ 3],
            "</TABLE>");

   len = 0;
   for( i = 0; i < 4; i++)
      len += strlen( tmp[ i]);

   // concatenate all strings

   s = malloc( len + 1);
   s[ 0] = 0;

   for( i = 0; i < 4; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   return( s);
}


char  *mulle_objc_staticstring_hor_html_description( struct _mulle_objc_staticstring *string)
{
   char           *s;
   
   asprintf( &s,
            "%s",
            html_escape( string->_s ? string->_s : "*null*"));
  
   return( s);
}


char  *mulle_objc_staticstring_html_row_description( void *value)
{
   struct _mulle_objc_staticstring *string = value;
   char   *s;
   
   asprintf( &s,
            "<TR><TD COLSPAN=\"2\">%s</TD></TR>\n",
            html_escape( string->_s ? string->_s : "*null*"));
   
   return( s);
}


#pragma mark - classes

char   *mulle_objc_class_short_html_description( struct _mulle_objc_class *cls)
{
   char   *s;
   char   *prefix;
   char   *name;
   
   name   = html_escape( cls->name);
   prefix = _mulle_objc_class_is_metaclass( cls) ? "+" : "";
   asprintf( &s, "<a href=\"%s%s.html\">%s (%08x)</a>\n",
            prefix,
            name,
            name,
            _mulle_objc_class_get_classid( cls));
   return( s);
}


char   *mulle_objc_class_html_row_description( intptr_t  classid,
                                                     void *cls)
{
   char   *s;
   char   *name;
   char   *prefix;
   
   name   = html_escape( _mulle_objc_class_get_name( cls));
   prefix = _mulle_objc_class_is_metaclass( cls) ? "+" : "";
   asprintf( &s, "<TR><TD>%08x</TD><TD><A HREF=\"%s%s.html\">%s</A></TD></TR>\n",
            _mulle_objc_class_get_classid( cls),
            prefix,
            name,
            name);
   return( s);
}


char   *mulle_objc_class_html_description( struct _mulle_objc_class *cls, char *color)
{
   size_t         len;
   char           *s;
   char           *tmp[ 7];
   unsigned int   i, n;
   char           *name;
   
   name = html_escape( cls->name);

   i = 0;
   // create single lines for each method and two for head/tail
   asprintf( &tmp[ i++],
            "<TABLE>\n<TR><TD BGCOLOR=\"%s\" COLSPAN=\"2\"><FONT COLOR=\"white\">%s</FONT></TD></TR>\n",
            color, name);
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

   len = 0;
   n   = i;
   for( i = 0; i < n; i++)
      len += strlen( tmp[ i]);

   // concatenate all strings

   s = malloc( len + 1);
   s[ 0] = 0;

   for( i = 0; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   return( s);
}


#pragma mark - ivarlist

char   *mulle_objc_ivarlist_html_description( struct _mulle_objc_ivarlist *list)
{
   size_t         len;
   char           **tmp;
   char           *s;
   unsigned int   i;
   unsigned int   n;
   
   n    = list->n_ivars + 2;
   tmp  = calloc( n, sizeof( char *));
   if( ! tmp)
      return(NULL);

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0], "<TABLE>\n");
   len = strlen( tmp[ 0]);

   for( i = 0; i < list->n_ivars; i++)
   {
      asprintf( &tmp[ i + 1], "<TR><TD>%s</TD>"
               "<TD><TABLE>"
               "<TR><TD>signature</TD><TD>%s</TD></TR>"
               "<TR><TD>methodid</TD><TD>0x%lx</TD></TR>"
               "<TR><TD>offset</TD><TD>%d</TD></TR>"
               "</TABLE></TD>"
               "</TR>\n",
               html_escape( list->ivars[ i].descriptor.name),
               html_escape( list->ivars[ i].descriptor.signature),
               (long) list->ivars[ i].descriptor.ivarid,
               list->ivars[ i].offset);
      len += strlen( tmp[ i + 1]);
   }

   asprintf( &tmp[ i + 1], "</TABLE>");
   len += strlen( tmp[ i + 1]);


   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}


char   *mulle_objc_ivarlist_html_hor_description( struct _mulle_objc_ivarlist *list)
{
   size_t         len;
   char           **tmp;
   char           *s;
   unsigned int   i;
   unsigned int   n;

   n   = list->n_ivars + 2;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return(NULL);

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0], "<TABLE>\n");
   len = strlen( tmp[ 0]);

   for( i = 0; i < list->n_ivars; i++)
   {
      asprintf( &tmp[ i + 1], "<TR><TD>%s</TD>"
               "<TD>%s</TD>"
               "<TD>%08x</TD>"
               "<TD>%d</TD>"
               "</TR>\n",
               html_escape( list->ivars[ i].descriptor.name),
               html_escape( list->ivars[ i].descriptor.signature),
               list->ivars[ i].descriptor.ivarid,
               list->ivars[ i].offset);
      len += strlen( tmp[ i + 1]);
   }

   asprintf( &tmp[ i + 1], "</TABLE>");
   len += strlen( tmp[ i + 1]);


   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}


#pragma mark - methoddescriptor


char   *mulle_objc_methoddescriptor_html_description( intptr_t methodid, struct _mulle_objc_methoddescriptor *desc)
{
   char   *s;

   asprintf( &s,
            "<TABLE>"
            "<TR><TD>name</TD><TD>%s</TD></TR>"
            "<TR><TD>signature</TD><TD>%s</TD></TR>"
            "<TR><TD>methodid</TD><TD>%08x</TD></TR>"
            "<TR><TD>bits</TD><TD>0x%x</TD></TR>"
            "</TABLE>",
            html_escape( desc->name),
            html_escape( desc->signature),
            desc->methodid,
            desc->bits);

   return( s);
}


char   *mulle_objc_methoddescriptor_html_hor_description( struct _mulle_objc_methoddescriptor *desc)
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


char   *mulle_objc_methoddescriptor_html_row_description( intptr_t  methodid,
                                                          void *value)
{
   struct _mulle_objc_methoddescriptor *desc = value;
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


#pragma mark - propertylist

char   *mulle_objc_propertylist_html_description( struct _mulle_objc_propertylist *list)
{
   size_t         len;
   char           **tmp;
   char           *s;
   unsigned int   i;
   unsigned int   n;

   n   = list->n_properties + 2;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return( NULL);

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0], "<TABLE>\n");
   len = strlen( tmp[ 0]);

   for( i = 0; i < list->n_properties; i++)
   {
      asprintf( &tmp[ i + 1], "<TR><TD>%s</TD>"
               "<TD><TABLE>"
               "<TR><TD>signature</TD><TD>%s</TD></TR>"
               "<TR><TD>propertyid</TD><TD>%08x</TD></TR>"
               "<TR><TD>getter</TD><TD>%08x</TD></TR>"
               "<TR><TD>setter</TD><TD>%08x</TD></TR>"
               "<TR><TD>clearer</TD><TD>%08x</TD></TR>"
               "</TABLE></TD>"
               "</TR>\n",
               html_escape( list->properties[ i].name),
               html_escape( list->properties[ i].signature),
               list->properties[ i].propertyid,
               list->properties[ i].getter,
               list->properties[ i].setter,
               list->properties[ i].clearer);

      len += strlen( tmp[ i + 1]);
   }

   asprintf( &tmp[ i + 1], "</TABLE>");
   len += strlen( tmp[ i + 1]);

   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}


#pragma mark - cache

char   *mulle_objc_cache_html_description( struct _mulle_objc_cache *cache)
{
   size_t          len;
   char            **tmp;
   char            *s;
   unsigned long   i;
   unsigned long   n;

   n   = cache->size + 2;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return(NULL);

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0],
             "<TABLE>\n<TR><TD BGCOLOR=\"gray\" COLSPAN=\"2\">size</TD><TD>%lu</TD></TR>\n",
             (long) cache->size);
   len = strlen( tmp[ 0]);

   for( i = 0; i < cache->size; i++)
   {
      asprintf( &tmp[ i + 1],
               "<TR><TD>#%ld</TD><TD>0x%lx</TD><TD>%p</TD></TR>\n",
               i,
               (long) cache->entries[ i].key.uniqueid,
               cache->entries[ i].value.functionpointer);
      len += strlen( tmp[ i + 1]);
   }

   asprintf( &tmp[ i + 1], "</TABLE>");
   len += strlen( tmp[ i + 1]);

   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}


#pragma mark - methodlist


char   *mulle_objc_methodlist_html_description( struct _mulle_objc_methodlist *list)
{
   size_t         len;
   char           **tmp;
   char           *s;
   unsigned int   i;
   unsigned int   n;

   n   = list->n_methods + 2;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return(NULL);

   // create single lines for each method and two for head/tail
   asprintf( &tmp[ 0], "<TABLE>\n");
   len = strlen( tmp[ 0]);

   for( i = 0; i < list->n_methods; i++)
   {
      asprintf( &tmp[ i + 1], "<TR><TD>%s</TD>"
               "<TD><TABLE>"
               "<TR><TD>signature</TD><TD>%s</TD></TR>"
               "<TR><TD>methodid</TD><TD>0x%lx</TD></TR>"
               "<TR><TD>bits</TD><TD>0x%x</TD></TR>"
               "<TR><TD>implementation</TD><TD>%p</TD></TR>"
               "</TABLE></TD>"
               "</TR>\n",
               html_escape( list->methods[ i].descriptor.name),
               html_escape( list->methods[ i].descriptor.signature),
               (long) list->methods[ i].descriptor.methodid,
               list->methods[ i].descriptor.bits,
               list->methods[ i].implementation);
      len += strlen( tmp[ i + 1]);
   }

   asprintf( &tmp[ i + 1], "</TABLE>");
   len += strlen( tmp[ i + 1]);

   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}


char   *mulle_objc_methodlist_html_hor_description( struct _mulle_objc_methodlist *list)
{
   size_t         len;
   char           **tmp;
   char           *s;
   unsigned int   i;
   unsigned int   j;
   unsigned int   n;

   n   = list->n_methods + 3;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return( NULL);

   // create single lines for each method and two for head/tail
   j = 0;
   asprintf( &tmp[ j],
             "<TABLE>\n<TR>\n",
             list->n_methods);
   len = strlen( tmp[ j]);

   if( list->n_methods)
   {
      ++j;
      asprintf( &tmp[ j], "<TR><TH>name</TH>"
                          "<TH>signature</TH>"
                          "<TH>methodid</TH>"
                          "<TH>bits</TH>"
                          "<TH>implementation</TH>"
                           "</TR>\n");
      len += strlen( tmp[ j]);
   }

   for( i = 0; i < list->n_methods; i++)
   {
      ++j;
      asprintf( &tmp[ j], "<TR><TD>%s</TD>"
               "<TD>%s</TD>"
               "<TD>%08x</TD>"
               "<TD>0x%x</TD>"
               "<TD>%p</TD>"
               "</TR>\n",
               html_escape( list->methods[ i].descriptor.name),
               html_escape( list->methods[ i].descriptor.signature),
               list->methods[ i].descriptor.methodid,
               list->methods[ i].descriptor.bits,
               list->methods[ i].implementation);
      len += strlen( tmp[ j]);
   }

   ++j;
   asprintf( &tmp[ j], "</TABLE>");
   len += strlen( tmp[ j]);

   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   for( i = 1; i <= j; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}

#pragma mark - loadclass

char   *mulle_objc_loadclass_html_row_description( void *value)
{
   struct _mulle_objc_loadclass   *loadcls = value;
   
   char   *s;
   asprintf( &s, "<TR><TD BGCOLOR=\"indigo\"><FONT COLOR=\"white\">%08x</FONT></TD><TD>%s</TD></TR>\n",
            loadcls->classid,
            loadcls->classname);
   return( s);
}


#pragma mark - loadcategory

char   *mulle_objc_loadcategory_html_row_description( void *value)
{
   struct _mulle_objc_loadcategory   *loadcat = value;
   
   char   *s;
   asprintf( &s, "<TR><TD BGCOLOR=\"darkslateblue\"><FONT COLOR=\"white\">%08x</FONT></TD><TD>%s( %s)</TD></TR>\n",
            loadcat->categoryid,
            loadcat->classname,
            loadcat->categoryname);
   return( s);
}



#pragma mark - protocols

char   *mulle_objc_protocols_html_description( struct   mulle_concurrent_pointerarray *array,
                                               struct _mulle_objc_colored_string *title)
{
   return( mulle_concurrent_pointerarray_html_description( array,
                                                           uniqueid_html_row_description,
                                                           title));
}


#pragma mark - categories

char   *mulle_objc_categories_html_description( struct   mulle_concurrent_pointerarray *array,
                                                     struct _mulle_objc_colored_string *title)
{
   return( mulle_concurrent_pointerarray_html_description( array,
                                                           uniqueid_html_row_description,
                                                           title));
}


#pragma mark - pointerarray

char   *mulle_concurrent_pointerarray_html_description( struct   mulle_concurrent_pointerarray *list,
                                                       char *(row_description)( void *),
                                                       struct _mulle_objc_colored_string *title)

{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   size_t                  count;
   size_t                  len;
   char                    **tmp;
   char                    *s;
   unsigned int            i;
   unsigned int            n;
   void                    *value;
   char                    *null_description;
   
   count = mulle_concurrent_pointerarray_get_count( list);

   n   = (unsigned int) count + 2;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return( NULL);

   tmp[ 0] = NULL;
   
   i   = 0;
   len = 0;
   if( title)
   {
      asprintf( &tmp[ i],
               "<TABLE>\n"
               "<TR><TD BGCOLOR=\"%s\" COLSPAN=\"2\"><FONT COLOR=\"%s\">%s</FONT></TD></TR>\n",
               title->backgroundColor,
               title->color,
               title->text);
      len += strlen( tmp[ i]);
      ++i;
   }

   null_description = "*null*";

   rover = mulle_concurrent_pointerarray_enumerate( list);
   while( value = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( ! value)
         value = null_description;
      tmp[ i] = row_description( value);
      len    += strlen( tmp[ i]);
      ++i;
   }

   if( title)
   {
      asprintf( &tmp[ i], "</TABLE>");
      len += strlen( tmp[ i]);
      ++i;
   }
   
   n = i;
   
   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   s[ len] = 0; // for i == 0 case
   
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}

#pragma mark - hashmap
char   *mulle_concurrent_hashmap_html_description( struct mulle_concurrent_hashmap *map,
                                                   char *(row_description)( intptr_t, void *),
                                                   struct _mulle_objc_colored_string *title)
{
   struct mulle_concurrent_hashmapenumerator   rover;
   char                    *null_description;
   intptr_t                uniqueid;
   size_t                  count;
   size_t                  len;
   char                    **tmp;
   char                    *s;
   unsigned int            i;
   unsigned int            n;
   void                    *value;


   count = mulle_concurrent_hashmap_count( map);

   n   = (unsigned int) count + 2;
   tmp = calloc( n, sizeof( char *));
   if( ! tmp)
      return(NULL);

   i   = 0;
   len = 0;
   if( title)
   {
      asprintf( &tmp[ i],
               "<TABLE>\n"
               "<TR><TD BGCOLOR=\"%s\" COLSPAN=\"2\"><FONT COLOR=\"%s\">%s</FONT></TD></TR>\n",
               title->backgroundColor,
               title->color,
               title->text);
      
      len += strlen( tmp[ i]);
      ++i;
   }
   
   null_description = "*null*";

   rover = mulle_concurrent_hashmap_enumerate( map);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &uniqueid, &value))
   {
      if( ! value)
         value = null_description;
      tmp[ i] = row_description( uniqueid, value);
      len    += strlen( tmp[ i]);
      ++i;
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   if( title)
   {
      asprintf( &tmp[ i], "</TABLE>");
      len += strlen( tmp[ i]);
      ++i;
   }
   
   n = i;

   // concatenate all strings

   s = realloc( tmp[ 0], len + 1);
   s[ len] = 0; // for i == 0 case
   for( i = 1; i < n; i++)
   {
      strcat( s, tmp[ i]);
      free( tmp[ i]);
   }

   free( tmp);

   return( s);
}

