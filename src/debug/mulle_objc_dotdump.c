//
//  mulle_objc_dotdump.c
//  mulle-objc-runtime
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
#include "mulle_objc_dotdump.h"

#include "mulle_objc_runtime.h"
#include "mulle_objc_html.h"
#include "mulle_objc_universe.h"  // for getenv

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_concurrent/mulle_concurrent.h>

#include "c_set.inc"


static char   *html_escape( char *s)
{
   if( ! strchr( s, '&') && ! strchr( s, '<'))
      return( s);

   return( "bad-html");
}


static char   *dot_filename_for_name( char *name, char *directory)
{
   char     *buf;
   size_t   len;

   len = strlen( name) + strlen( directory) + 16;
   buf = mulle_allocator_malloc( &mulle_stdlib_allocator, len);
   sprintf( buf, "%s/%s.dot", directory, html_escape( name));
   return( buf);
}



//
// just don't output stuff with ampersands for now
// What is this used for ?
//
void   mulle_objc_methodlist_dump( struct _mulle_objc_methodlist *list)
{
   unsigned int   i;
   char           buf[ s_mulle_objc_sprintf_functionpointer_buffer];

   for( i = 0; i < list->n_methods; i++)
   {
      mulle_objc_sprintf_functionpointer( buf,
         _mulle_atomic_functionpointer_nonatomic_read( &list->methods[ i].implementation));

      printf( "{ "
              "name = \"%s\""
              "signature = \"%s\""
              "methodid = %08x"
              "bits = 0x%x"
              "implementation = %s"
              " }",
               list->methods[ i].descriptor.name,
               list->methods[ i].descriptor.signature,
               list->methods[ i].descriptor.methodid,
               list->methods[ i].descriptor.bits,
               buf);
   }
}


# pragma mark - "styling"

static struct _mulle_objc_htmltablestyle    infraclass_style =
{
   "infraclass",
   NULL,
   "white",
   "blue",
   0
};


static struct _mulle_objc_htmltablestyle    metaclass_style =
{
   "metaclass",
   NULL,
   "white",
   "goldenrod",
   0
};



static struct _mulle_objc_htmltablestyle    methodlist_style =
{
   "methodlist",
   NULL,
   "white",
   "black",
   0
};


static struct _mulle_objc_htmltablestyle    cachetable_style =
{
   "cache",
   NULL,
   "white",
   "black",
   0
};


static struct _mulle_objc_htmltablestyle    ivarlist_style =
{
   "ivarlist",
   NULL,
   "white",
   "black",
   0
};


static struct _mulle_objc_htmltablestyle    propertylist_style =
{
   "propertylist",
   NULL,
   "white",
   "black",
   0
};


static struct _mulle_objc_htmltablestyle    selectortable_style =
{
   "method descriptors",
   NULL,
   "white",
   "purple",
   5
};


static struct _mulle_objc_htmltablestyle    staticstringtable_title =
{
   "strings",
   NULL,
   "black",
   "chartreuse",
   0
};



static struct _mulle_objc_htmltablestyle    fastclasstable_title =
{
   "fastclasses",
   NULL,
   "white",
   "blue",
   0
};


static struct _mulle_objc_htmltablestyle  classtable_style =
{
   "classes",
   NULL,
   "white",
   "blue",
   0
};


static struct _mulle_objc_htmltablestyle    categorytable_style =
{
   "categories",
   NULL,
   "white",
   "dimgray",
   0
};


static struct _mulle_objc_htmltablestyle    supertable_style =
{
   "supers",
   NULL,
   "white",
   "dimgray",
   0
};


static struct _mulle_objc_htmltablestyle    protocoltable_style =
{
   "protocols",
   NULL,
   "white",
   "dimgray",
   0
};


static struct _mulle_objc_htmltablestyle    classestoload_style =
{
   "classes to load",
   NULL,
   "white",
   "indigo",
   0
};


static struct _mulle_objc_htmltablestyle    categoriestoload_style =
{
   "categories to load",
   NULL,
   "white",
   "indigo",
   0
};


static struct _mulle_objc_htmltablestyle    universe_style =
{
   "universe",
   NULL,
   "white",
   "red",
   0
};


# pragma mark - walker universe callback

static char  *mulle_objc_loadclasslist_html_row_description( intptr_t  classid, void *value,                                                     struct _mulle_objc_htmltablestyle *styling)
{
   struct mulle_concurrent_pointerarray   *array = value;

   return( mulle_concurrent_pointerarray_html_description( array,
                                                           mulle_objc_loadclass_html_row_description,
                                                           NULL));
}


static char  *mulle_objc_loadcategorylist_html_row_description( intptr_t  classid, void *value,                                                     struct _mulle_objc_htmltablestyle *styling)
{
   struct mulle_concurrent_pointerarray   *array = value;

   return( mulle_concurrent_pointerarray_html_description( array,
                                                           mulle_objc_loadcategory_html_row_description,
                                                           NULL));
}


struct dump_info
{
   c_set  set;
   FILE   *fp;
   char   *directory;
   char   create_hyperlink;
   char   protocolclasses_as_hyperlink;
   char   universe_as_hyperlink;
   char   draw_strings;  // usually boring
   char   omit_selectors;  // boring for movies
};


static void   print_universe( struct _mulle_objc_universe *universe,
                              struct dump_info *info)
{
   char   *label;
   int    i;

   label = mulle_objc_universe_html_description( universe, &universe_style);
   fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\", URL=\"file:///%s/overview.dot\"  ];\n", universe, label, "component", info->directory);
   free( label);
   
   if( ! info->omit_selectors)
      if( mulle_concurrent_hashmap_count( &universe->descriptortable))
      {
         fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"descriptortable\" ];\n",
                 universe, &universe->descriptortable);
         
         label = mulle_concurrent_hashmap_html_description( &universe->descriptortable,
                                                           mulle_objc_descriptor_html_row_description,
                                                           &selectortable_style);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
                 &universe->descriptortable, label, "box");
         free( label);
      }
   
   if( mulle_concurrent_hashmap_count( &universe->protocoltable))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"protocoltable\" ];\n",
              universe, &universe->protocoltable);
      
      label = mulle_concurrent_hashmap_html_description( &universe->protocoltable,
                                                        mulle_objc_protocol_html_row_description,
                                                        &protocoltable_style);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &universe->protocoltable, label, "box");
      free( label);
   }
   
   if( mulle_concurrent_hashmap_count( &universe->categorytable))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"categorytable\" ];\n",
              universe, &universe->categorytable);
      
      label = mulle_concurrent_hashmap_html_description( &universe->categorytable,
                                                        mulle_objc_category_html_row_description,
                                                        &categorytable_style);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &universe->categorytable, label, "box");
      free( label);
   }

   if( mulle_concurrent_hashmap_count( &universe->supertable))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"supertable\" ];\n",
              universe, &universe->supertable);
      
      label = mulle_concurrent_hashmap_html_description( &universe->supertable,
                                                        mulle_objc_super_html_row_description,
                                                        &supertable_style);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &universe->supertable, label, "box");
      free( label);
   }

   if( info->draw_strings)
      if( mulle_concurrent_pointerarray_get_count( &universe->staticstrings))
      {
         label = mulle_concurrent_pointerarray_html_description( &universe->staticstrings,
                                                                 mulle_objc_staticstring_html_row_description,
                                                                 &staticstringtable_title);

         fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"staticstrings\" ];\n",
                 universe, &universe->staticstrings);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
                 &universe->staticstrings, label, "box");
         free( label);
      }

   {
      for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
      {
         if( _mulle_atomic_pointer_nonatomic_read( &universe->fastclasstable.classes[ i].pointer))
            break;
      }
   
      if( i < MULLE_OBJC_S_FASTCLASSES)
      {
         label = mulle_objc_fastclasstable_html_description( &universe->fastclasstable,
                                                             mulle_objc_fastclassentry_html_row_description,
                                                             &fastclasstable_title);

         fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"fastclasses\" ];\n",
                 universe, &universe->fastclasstable);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
                 &universe->fastclasstable, label, "box");
         free( label);
      }
   }

   if( mulle_concurrent_hashmap_count( &universe->classtable))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"classes\" ];\n",
              universe, &universe->classtable);
      
      label = mulle_concurrent_hashmap_html_description( &universe->classtable,
                                                         mulle_objc_infraclass_html_row_description,
                                                         &classtable_style);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &universe->classtable, label, "box");
      free( label);
   }

   {
      if( mulle_concurrent_hashmap_count( &universe->waitqueues.classestoload))
      {
         fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"classestoload\" ];\n",
                 universe, &universe->waitqueues.classestoload);
         
         label = mulle_concurrent_hashmap_html_description( &universe->waitqueues.classestoload,
                                                           mulle_objc_loadclasslist_html_row_description,
                                                           &classestoload_style);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
                 &universe->waitqueues.classestoload, label, "box");
         free( label);
      }
      
      if( mulle_concurrent_hashmap_count( &universe->waitqueues.categoriestoload))
      {
         fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"categoriestoload\" ];\n",
                 universe, &universe->waitqueues.categoriestoload);
         
         label = mulle_concurrent_hashmap_html_description( &universe->waitqueues.categoriestoload,
                                                           mulle_objc_loadcategorylist_html_row_description,
                                                           &categoriestoload_style);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
                 &universe->waitqueues.categoriestoload, label, "box");
         free( label);
      }
   }
   fprintf( info->fp, "\n\n");
}


static void   print_hyper_universe( struct _mulle_objc_universe *universe,
                                    struct dump_info *info)
{
   char   *label;

   label = mulle_objc_universe_html_description( universe, &universe_style);
   fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\", URL=\"file:///%s/universe.dot\"  ];\n", universe, label, "component", info->directory);
   free( label);
}


# pragma mark - walker class callback


static void   print_infraclass( struct _mulle_objc_infraclass *infra,
                                struct dump_info *info);
static void   print_metaclass( struct _mulle_objc_metaclass *meta,
                              struct dump_info *info);
static void   print_hyper_infraclass( struct _mulle_objc_infraclass *infra,
                                      struct dump_info *info);

extern char   *_mulle_objc_grapviz_html_header_description( char *name, int is_meta);

static void   print_class( struct _mulle_objc_class *cls,
                           struct dump_info *info,
                           int is_meta)
{
   char                                             *label;
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_cache                         *cache;
   struct _mulle_objc_class                         *superclass;
   struct _mulle_objc_classpair                     *pair;
   struct _mulle_objc_infraclass                    *infra;
   struct _mulle_objc_metaclass                     *meta;
   struct _mulle_objc_methodlist                    *methodlist;
   struct _mulle_objc_universe                      *universe;
   unsigned int                                     i;
   struct _mulle_objc_htmltablestyle                style;

   style       = is_meta ? metaclass_style : infraclass_style;
   style.title = cls->name;

   label = mulle_objc_class_html_description( cls, &style);
   fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\"", cls, label, is_meta ? "component" : "box");
   free( label);

   // always create this, what's the harm
   {
      //
      // graphviz seemingly can only open absolute path links.
      // For svg you need to rewrite it anyway to .svg extension
      // so cut off the "file:///tmp/" prefix there with
      // `sed 's|URL=\"file:///tmp/\([^"]*\).dot\"|URL=\"\1.svg\"|'`
      // batch convert:
      // for i in *.dot;
      // do
      //    name="`basename -- "$i" .dot`"
      //    sed 's|URL=\"file:///tmp/\([^"]*\).dot\"|URL=\"\1.svg\"|' "$i" | dot -Tsvg -o "${name}.svg"
      //  done
      // Note: URL must be uppercase
      fprintf( info->fp, " URL=\"file:///%s/%s.dot\"", info->directory, html_escape( cls->name));
   }
   fprintf( info->fp, " ];\n");

   universe = _mulle_objc_class_get_universe( cls);
   if( universe)
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"universe\" ];\n", cls,  _mulle_objc_class_get_universe( cls));

   // meta superclass is boring
   if( ! is_meta)
   {
      superclass = _mulle_objc_class_get_superclass( cls);
      if( superclass)
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"super\"; penwidth=\"%d\" ];\n", cls, superclass,
                 _mulle_objc_class_is_infraclass( cls)
                 ? 3 : 1);
   }

   {
      meta = _mulle_objc_class_get_metaclass( cls);
      if( meta)
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"meta\"; color=\"goldenrod\"; fontcolor=\"goldenrod\" ];\n", cls, meta);
   }

   {
      infra = _mulle_objc_class_get_infraclass( cls);
      if( infra)
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"infra\"; color=\"blue\"; fontcolor=\"blue\" ];\n", cls, infra);
   }

   {
      if( ! (_mulle_objc_class_get_inheritance( cls) & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
      {
         struct _mulle_objc_protocolclassenumerator  rover;
         struct _mulle_objc_infraclass               *prop_cls;

         i     = 0;
         pair  = _mulle_objc_class_get_classpair( cls);
         rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
         while( prop_cls = _mulle_objc_protocolclassenumerator_next( &rover))
         {
            fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"protocol class #%u\" ];\n",
                    cls, prop_cls, i++);

            if( ! c_set_member( &info->set, prop_cls))
            {
               c_set_add( &info->set, prop_cls);
               if( info->protocolclasses_as_hyperlink)
                  print_hyper_infraclass( prop_cls, info);
               else
               {
                  print_infraclass( prop_cls, info);
                  print_metaclass( _mulle_objc_infraclass_get_metaclass( prop_cls), info);
               }
            }
         }
         _mulle_objc_protocolclassenumerator_done( &rover);
      }
      fprintf( info->fp, "\n\n");
   }

   {
      i = 0;
      rover = mulle_concurrent_pointerarray_enumerate( &cls->methodlists);
      while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
      {
         if( methodlist->n_methods)
         {
            fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"methodlist #%d\" ];\n",
                    cls, methodlist, i++);

            label = mulle_objc_methodlist_html_description( methodlist,
                                                            universe,
                                                            &methodlist_style);
            fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", methodlist, label);
            free( label);
         }
      }
      mulle_concurrent_pointerarrayenumerator_done( &rover);
   }

   {
      cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
      if( _mulle_atomic_pointer_nonatomic_read( &cache->n))
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"cache\" ];\n",
                 cls, cache);

         label = mulle_objc_cache_html_description( cache, &cachetable_style);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", cache, label);
         free( label);
      }
   }
}


static void   print_classpair( struct _mulle_objc_classpair *pair,
                               struct _mulle_objc_class *cls,
                               struct dump_info *info)
{
   struct _mulle_objc_uniqueidarray   *array;
   char                               *label;
   struct _mulle_objc_universe        *universe;
   
   universe = _mulle_objc_classpair_get_universe( pair);
   
   array = _mulle_atomic_pointer_read( &pair->p_protocolids.pointer);
   if( array->n)
   {
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"protocolids\" ];\n",
              cls, array);

      label = mulle_objc_protocols_html_description( array,
                                                     universe,
                                                     &protocoltable_style);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", array, label);
      free( label);
   }

   array = _mulle_atomic_pointer_read( &pair->p_categoryids.pointer);
   if( array->n)
   {
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"categoryids\" ];\n",
              cls, array);

      label = mulle_objc_categories_html_description( array,
                                                      universe,
                                                      &categorytable_style);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", array, label);
      free( label);
   }
}


static void   print_infraclass( struct _mulle_objc_infraclass *infra,
                                struct dump_info *info)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   char                                             *label;
   struct _mulle_objc_ivarlist                      *ivarlist;
   struct _mulle_objc_propertylist                  *propertylist;
   unsigned int                                     i;

   print_class( _mulle_objc_infraclass_as_class( infra), info, 0);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &infra->ivarlists);
   while( ivarlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( ivarlist->n_ivars)
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"ivarlist #%d\" ];\n",
                 infra, ivarlist, i++);

         label = mulle_objc_ivarlist_html_description( ivarlist, &ivarlist_style);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", ivarlist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &infra->propertylists);
   while( propertylist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( propertylist->n_properties)
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"propertylist #%d\" ];\n",
                 infra, propertylist, i++);

         label = mulle_objc_propertylist_html_description( propertylist, &propertylist_style);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", propertylist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   print_classpair( _mulle_objc_infraclass_get_classpair( infra),
                    _mulle_objc_infraclass_as_class( infra),
                    info);
}


static void   print_hyper_infraclass( struct _mulle_objc_infraclass *infra,
                                      struct dump_info *info)
{
   struct _mulle_objc_htmltablestyle   style;
   char                                *label;
   struct _mulle_objc_infraclass       *superclass;


   style       = infraclass_style;
   style.title = html_escape( infra->base.name);

   label = mulle_objc_class_html_tiny_description( _mulle_objc_infraclass_as_class( infra), &style);

   fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"box\", URL=\"file:///%s/%s.dot\" ];\n", infra, label, info->directory, style.title);
   free( label);

   if( _mulle_objc_infraclass_get_universe( infra))
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"universe\" ];\n", infra,  _mulle_objc_infraclass_get_universe( infra));
   
   {
      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass)
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"super\" ];\n",
                 infra, superclass);
   }
}



static void   print_metaclass( struct _mulle_objc_metaclass *meta,
                               struct dump_info *info)
{
   print_class( _mulle_objc_metaclass_as_class( meta), info, 1);

   print_classpair( _mulle_objc_metaclass_get_classpair( meta),
                    _mulle_objc_metaclass_as_class( meta),
                    info);
}


static int   callback( struct _mulle_objc_universe *universe,
                       void *p,
                       enum mulle_objc_walkpointertype_t type,
                       char *key,
                       void *parent,
                       void *userinfo)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct dump_info                *info;

   assert( p);

   if( key)
      return( mulle_objc_walk_ok);

   info = userinfo;

   if( c_set_member( &info->set, p))
      return( mulle_objc_walk_dont_descend);
   c_set_add( &info->set, p);

   switch( type)
   {
   case mulle_objc_walkpointer_is_category  :
   case mulle_objc_walkpointer_is_protocol  :
   case mulle_objc_walkpointer_is_classpair :
      break;

   case mulle_objc_walkpointer_is_universe  :
      universe = p;
      if( info->create_hyperlink)
         print_hyper_universe( universe, info);
      else
         print_universe( universe, info);
      break;

   case mulle_objc_walkpointer_is_infraclass :
      infra = p;
      if( info->create_hyperlink)
         print_hyper_infraclass( infra, info);
      else
         print_infraclass( infra, info);
      break;

   case mulle_objc_walkpointer_is_metaclass :
      meta = p;
      if( ! info->create_hyperlink)
         print_metaclass( meta, info);
      break;

   case mulle_objc_walkpointer_is_method :
   case mulle_objc_walkpointer_is_property :
   case mulle_objc_walkpointer_is_ivar :
      break;
   }

   return( mulle_objc_walk_ok);
}


# pragma mark - class dump

static void   _mulle_objc_class_dotdump_to_file( struct _mulle_objc_class *cls,
                                                 char *directory,
                                                 char *filename)
{
   FILE                            *fp;
   struct _mulle_objc_classpair    *pair;
   struct dump_info                info;

   memset( &info, 0, sizeof( info));

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   fprintf( fp, "digraph mulle_objc_class\n{\n");

   c_set_init( &info.set);
   info.fp        = fp;
   info.directory = directory;
   
   info.protocolclasses_as_hyperlink = 1;
   info.universe_as_hyperlink        = 1;
   
   print_hyper_universe( _mulle_objc_class_get_universe( cls), &info);
   
   pair = NULL;
   do
   {
      pair = _mulle_objc_class_get_classpair( cls);
      mulle_objc_classpair_walk( pair, callback, &info);

      info.create_hyperlink = 1;
   }
   while( cls = _mulle_objc_class_get_superclass( cls));

   fprintf( fp, "}\n");
   fclose( fp);

   c_set_done( &info.set);
}


static void   mulle_objc_class_dotdump_to_file( struct _mulle_objc_class *cls,
                                                char *directory,
                                                char *filename)
{
   _mulle_objc_class_dotdump_to_file( cls, directory, filename);
   fprintf( stderr, "Written dot file \"%s\"\n", filename);
}


static void   _mulle_objc_dotdump_classname_to_file( char *classname,
                                                     char  *directory,
                                                     char *filename)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_infraclass   *infra;
   mulle_objc_classid_t            classid;

   if( ! classname || ! *classname)
   {
      fprintf( stderr, "Invalid classname\n");
      return;
   }

   universe = mulle_objc_get_universe();
   classid = mulle_objc_classid_from_string( classname);
   infra   = _mulle_objc_universe_fastlookup_infraclass( universe, classid);
   if( ! infra)
   {
      fprintf( stderr, "Class \"%s\" is unknown to the universe\n", classname);
      return;
   }

   cls = _mulle_objc_infraclass_as_class( infra);
   _mulle_objc_class_dotdump_to_file( cls, directory, filename);
}


static void   mulle_objc_dotdump_classname_to_file( char *classname,
                                                    char *directory,
                                                    char *filename)
{
   _mulle_objc_dotdump_classname_to_file( classname, directory, filename);
   fprintf( stderr, "Written dot file \"%s\"\n", filename);
}


static void   __mulle_objc_dotdump_classes( void (*dump)( char *, char *, char *),
                                            char *directory)
{
   char                                        *path;
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct _mulle_objc_universe                 *universe;
   struct mulle_concurrent_hashmapenumerator   rover;

   universe = mulle_objc_get_universe();
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      path = dot_filename_for_name( infra->base.name, directory);
      (*dump)( infra->base.name, directory, path);
      mulle_allocator_free( &mulle_stdlib_allocator, path);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}

# pragma mark - overview dump

static void   _mulle_objc_universe_dotdump_overview( struct _mulle_objc_universe *universe,
                                                    char *directory,
                                                    FILE *fp)
{
   struct dump_info   info;

   c_set_init( &info.set);

   memset( &info, 0, sizeof( info));

   info.fp               = fp;
   info.directory        = directory;
   info.create_hyperlink = 1;

   fprintf( fp, "digraph mulle_objc_universe\n{\n");
   mulle_objc_universe_walk( universe, callback, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


static void   _mulle_objc_dotdump_overview_to_file( char *directory, char *filename)
{
   struct _mulle_objc_universe   *universe;
   FILE                         *fp;

   universe = mulle_objc_inlined_get_universe();
   if( ! universe)
   {
      fprintf( stderr, "No universe found!\n");
      return;
   }

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_universe_dotdump_overview( universe, directory, fp);
   fclose( fp);

   fprintf( stderr, "Written dot file \"%s\"\n", filename);
}


static void   _mulle_objc_dotdump_overview( char *directory)
{
   char   *path;
   
   path = dot_filename_for_name( "overview", directory);
   _mulle_objc_dotdump_overview_to_file( directory, path);
   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


# pragma mark - universe dump

static void   _mulle_objc_universe_dotdump( struct _mulle_objc_universe *universe,
                                           char *directory,
                                           FILE *fp)
{
   struct dump_info   info;

   c_set_init( &info.set);

   memset( &info, 0, sizeof( info));

   info.fp             = fp;
   info.directory      = directory;
   info.draw_strings   = (char) mulle_objc_getenv_yes_no_default( "MULLE_OBJC_DRAW_STRING_TABLE", 0);
   info.omit_selectors = ! (char) mulle_objc_getenv_yes_no_default( "MULLE_OBJC_DRAW_SELECTOR_TABLE", 1);
   
   fprintf( fp, "digraph mulle_objc_universe\n{\n");
   print_universe( universe, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


static void   mulle_objc_dotdump_universe_to_file( char *directory,
                                                   char *filename,
                                                   int log)
{
   struct _mulle_objc_universe   *universe;
   FILE                         *fp;

   universe = mulle_objc_inlined_get_universe();
   if( ! universe)
   {
      fprintf( stderr, "No universe found!\n");
      return;
   }

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_universe_dotdump( universe, directory, fp);
   fclose( fp);

   if( log)
      fprintf( stderr, "Written dot file \"%s\"\n", filename);
}


static void   _mulle_objc_dotdump_universe( char *directory, int log)
{
   char   *path;
   
   path = dot_filename_for_name( "universe", directory);
   mulle_objc_dotdump_universe_to_file( directory, path, log);
   mulle_allocator_free( &mulle_stdlib_allocator, path);
}

# pragma mark - classes dump

static void   _mulle_objc_dotdump_classes( char *directory)
{
   __mulle_objc_dotdump_classes( _mulle_objc_dotdump_classname_to_file, directory);
}


static void   _mulle_objc_class_dotdump( struct _mulle_objc_class *cls, char *directory)
{
   char   *path;
 
   if( ! cls)
      return;
   
   path = dot_filename_for_name( _mulle_objc_class_get_name( cls), directory);
   mulle_objc_class_dotdump_to_file( cls, directory, path);
   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


static void   _mulle_objc_dotdump_classname( char *classname, char *directory)
{
   char   *path;
   
   path = dot_filename_for_name( classname, directory);
   mulle_objc_dotdump_classname_to_file( classname, directory, path);
   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


#pragma mark - dump to /tmp

void   mulle_objc_dotdump_overview_to_tmp( void)
{
   _mulle_objc_dotdump_overview( "/tmp");
}


void   mulle_objc_dotdump_universe_to_tmp()
{
   _mulle_objc_dotdump_universe( "/tmp", 1);
}


void   mulle_objc_class_dotdump_to_tmp( struct _mulle_objc_class *cls)
{
   _mulle_objc_class_dotdump( cls, "/tmp");
}


void   mulle_objc_dotdump_classname_to_tmp( char *classname)
{
   _mulle_objc_dotdump_classname( classname, "/tmp");
}


void   mulle_objc_dotdump_classes_to_tmp( void)
{
   __mulle_objc_dotdump_classes( mulle_objc_dotdump_classname_to_file, "/tmp");
}


#pragma mark - dump to working directory

static char  *get_pwd( void)
{
   char   *pwd;
   
   // don't use OS specifica!
   pwd = getenv( "PWD");
   if( ! pwd)
      pwd = ".";
   return( pwd);
}

void   mulle_objc_dotdump_overview( void)
{
   _mulle_objc_dotdump_overview( get_pwd());
}


void   mulle_objc_dotdump_universe()
{
   _mulle_objc_dotdump_universe( get_pwd(), 1);
}


void   mulle_objc_class_dotdump( struct _mulle_objc_class *cls)
{
   char   *path;
   char   *directory;
   
   if( ! cls)
      return;
   
   directory = get_pwd();
   path = dot_filename_for_name( _mulle_objc_class_get_name( cls), get_pwd());
   mulle_objc_class_dotdump_to_file( cls, directory, path);
   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


void   mulle_objc_dotdump_classname( char *classname)
{
   char   *path;
   char   *directory;
   
   directory = get_pwd();
   path = dot_filename_for_name( classname, directory);
   mulle_objc_dotdump_classname_to_file( classname, directory, path);
   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


void   mulle_objc_dotdump_classes( void)
{
   __mulle_objc_dotdump_classes( mulle_objc_dotdump_classname_to_file, get_pwd());
}


#pragma mark - conveniences

void   mulle_objc_dotdump_universe_frame_to_tmp()
{
   static mulle_atomic_pointer_t   counter;
   auto char                       buf[ 32];
   int                             nr;
   int                             max;
   char                            *s;

   nr = (int) (intptr_t) _mulle_atomic_pointer_increment( &counter);
   s = getenv( "MULLE_OBJC_DOTDUMP_MAX");
   if( s)
   {
      max = atoi( s);
      if( max && nr >= max)
         return;
   }

   sprintf( buf, "/tmp/universe_%06d.dot", nr);
   mulle_objc_dotdump_universe_to_file( "/tmp", buf, 0);
}


void   mulle_objc_dotdump( void)
{
   char   *pwd;
   
   pwd = get_pwd();
   _mulle_objc_dotdump_overview( pwd);
   _mulle_objc_dotdump_universe( pwd, 0);
   _mulle_objc_dotdump_classes( pwd);
}


void   mulle_objc_dotdump_to_tmp( void)
{
   _mulle_objc_dotdump_overview( "/tmp");
   _mulle_objc_dotdump_universe( "/tmp", 0);
   _mulle_objc_dotdump_classes( "/tmp");
}
