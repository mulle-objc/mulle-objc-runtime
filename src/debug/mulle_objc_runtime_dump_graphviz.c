//
//  mulle_objc_runtime_dump_graphviz.c
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

#include "mulle_objc_runtime_dump_graphviz.h"

#include "mulle_objc.h"
#include "mulle_objc_html.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>


#include "c_set.inc"


//
// just don't output stuff with ampersands for now
//
void   mulle_objc_methodlist_dump( struct _mulle_objc_methodlist *list)
{
   unsigned int   i;

   for( i = 0; i < list->n_methods; i++)
      printf( "{ "
              "name = \"%s\""
              "signature = \"%s\""
              "methodid = 0x%lx"
              "bits = 0x%x"
              "implementation = %p"
              " }",
               list->methods[ i].descriptor.name,
               list->methods[ i].descriptor.signature,
               (long) list->methods[ i].descriptor.methodid,
               list->methods[ i].descriptor.bits,
               list->methods[ i].implementation);
}


# pragma mark - walker runtime callback

static void   print_runtime( struct _mulle_objc_runtime *runtime, FILE *fp)
{
   char  *label;
   int   i;
   struct _mulle_objc_staticstring           *string;
   struct mulle_concurrent_pointerarrayenumerator  rover;

   label = mulle_objc_runtime_html_description( runtime);
   fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", runtime, label, "component");
   free( label);

   fprintf( fp, "\"%p\" -> \"%p\" [ label=\"descriptortable\" ];\n", runtime, &runtime->descriptortable);

   label = mulle_concurrent_hashmap_html_description( &runtime->descriptortable, (void *) mulle_objc_methoddescriptor_html_description);
   fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", &runtime->descriptortable, label, "box");
   free( label);

   for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
      if( _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer))
         fprintf( fp, "\"%p\" -> \"%p\" [ label=\"fastclass #%d\" ];\n",
            runtime, _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer), i);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &runtime->staticstrings);
   while( string = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"string #%d\" ];\n",
              runtime, string, i++);

      label = mulle_objc_staticstring_html_description( string);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", string, label);
      free( label);

      if( _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( string)))
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"isa\" ];\n",
              string, _mulle_objc_object_get_isa( string));
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);
   fprintf( fp, "\n\n");
}


# pragma mark - walker class callback

struct dump_info
{
   c_set  set;
   FILE   *fp;
};


static void   print_class( struct _mulle_objc_class *cls, FILE *fp, int is_meta)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_protocolclassenumerator       prover;
   char                                             *label;
   struct _mulle_objc_cache                         *cache;
   struct _mulle_objc_class                         *superclass;
   struct _mulle_objc_class                         *propertyclass;
   struct _mulle_objc_ivarlist                      *ivarlist;
   struct _mulle_objc_methodlist                    *methodlist;
   struct _mulle_objc_propertylist                  *propertylist;
   unsigned int                                     i;

   label = mulle_objc_class_html_description( cls, is_meta ? "purple" : "blue");
   fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", cls, label, is_meta ? "component" : "box");
   free( label);

   if( _mulle_objc_class_get_runtime( cls))
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"runtime\" ];\n", cls,  _mulle_objc_class_get_runtime( cls));

   superclass = _mulle_objc_class_get_superclass( cls);
   if( superclass)
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"super\" ];\n", cls, superclass);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &cls->ivarlists);
   while( ivarlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"ivarlist #%d\" ];\n",
              cls, ivarlist, i++);

      label = mulle_objc_ivarlist_html_description( ivarlist);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", ivarlist, label);
      free( label);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &cls->propertylists);
   while( propertylist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"propertylist #%d\" ];\n",
              cls, propertylist, i++);

      label = mulle_objc_propertylist_html_description( propertylist);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", propertylist, label);
      free( label);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &cls->methodlists);
   while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"methodlist #%d\" ];\n",
              cls, methodlist, i++);

      label = mulle_objc_methodlist_html_description( methodlist);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", methodlist, label);
      free( label);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   if( ! (_mulle_objc_class_get_inheritance( cls) & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      i = 0;
      prover = _mulle_objc_class_enumerate_protocolclasses( cls);
      while( propertyclass = _mulle_objc_protocolclassenumerator_next( &prover))
      {
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"protocol inherit #%u\" ];\n",
                 cls, propertyclass, i++);
      }
      mulle_concurrent_pointerarrayenumerator_done( &rover);
   }
   fprintf( fp, "\n\n");

   if( mulle_concurrent_pointerarray_get_count( &cls->protocolids))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"protocolids\" ];\n",
              cls, &cls->protocolids);

      label = mulle_concurrent_pointerarray_html_description( &cls->protocolids);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", &cls->protocolids, label);
      free( label);
   }

   if( mulle_concurrent_pointerarray_get_count( &cls->categoryids))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"categoryids\" ];\n",
              cls, &cls->protocolids);

      label = mulle_concurrent_pointerarray_html_description( &cls->categoryids);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", &cls->categoryids, label);
      free( label);
   }

   cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"cache\" ];\n",
            cls, cache);

   label = mulle_objc_cache_html_description( cache);
   fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", cache, label);
   free( label);
}


static int   callback( struct _mulle_objc_runtime *runtime,
                       void *p,
                       enum mulle_objc_runtime_type_t type,
                       char *key,
                       void *parent,
                       void *userinfo)
{
   FILE                            *fp;
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_class        *infraclass;
   struct dump_info                *info;

   assert( p);

   if( key)
      return( mulle_objc_runtime_walk_ok);

   info = userinfo;
   fp   = info->fp;

   if( c_set_member( &info->set, p))
      return( mulle_objc_runtime_walk_dont_descend);
   c_set_add( &info->set, p);

   switch( type)
   {
   case mulle_objc_runtime_is_category :
   case mulle_objc_runtime_is_protocol :
      break;

   case mulle_objc_runtime_is_runtime  :
      runtime = p;
      print_runtime( runtime, fp);
      break;

   case mulle_objc_runtime_is_class :
      cls = p;
      print_class( cls, fp, 0);
      break;

   case mulle_objc_runtime_is_meta_class :
      cls = p;
      print_class( cls, fp, 1);
      if( parent)
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"meta\" ];\n", parent, cls);

      infraclass = _mulle_objc_class_get_infraclass( cls);
      if( infraclass)
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"infra\" ];\n", cls, infraclass);
      break;

   case mulle_objc_runtime_is_method :
   case mulle_objc_runtime_is_property :
   case mulle_objc_runtime_is_ivar :
      break;
   }

   return( mulle_objc_runtime_walk_ok);
}


# pragma mark - runtime dump


void   _mulle_objc_runtime_dump_graphviz( struct _mulle_objc_runtime *runtime, FILE *fp)
{
   struct dump_info  info;

   c_set_init( &info.set);
   info.fp = fp;

   fprintf( fp, "digraph mulle_objc_runtime\n{\n");
   mulle_objc_runtime_walk( runtime, callback, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


void   mulle_objc_runtime_locking_dump_graphviz( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
      return;

   if( ! _mulle_objc_runtime_trylock( runtime))
   {
      _mulle_objc_runtime_dump_graphviz( runtime, stdout);

      _mulle_objc_runtime_unlock( runtime);
   }
   else
      fprintf( stderr, "runtime locked\n");
}


void   mulle_objc_runtime_dump_graphviz( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
      return;

   _mulle_objc_runtime_dump_graphviz( runtime, stdout);
}


void   mulle_objc_runtime_dump_graphviz_to_file( char *filename)
{
   struct _mulle_objc_runtime   *runtime;
   FILE                         *fp;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
   {
      fprintf( stderr, "No runtime found!\n");
      return;
   }

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_runtime_dump_graphviz( runtime, fp);
   fclose( fp);

   fprintf( stderr, "Dumped \"%s\"\n", filename);
}


void   mulle_objc_runtime_dump_graphviz_tmp( void)
{
   mulle_objc_runtime_dump_graphviz_to_file( "/tmp/mulle_objc_runtime.dot");
}


# pragma mark - class dump


void   _mulle_objc_class_dump_graphviz( struct _mulle_objc_class *cls, FILE *fp)
{
   struct dump_info  info;
   extern mulle_objc_runtime_walkcommand_t
   mulle_objc_runtime_class_walk( struct _mulle_objc_runtime *,
                                 struct _mulle_objc_class *,
                                 enum mulle_objc_runtime_type_t,
                                 mulle_objc_runtime_walkcallback,
                                 void *,
                                 void *);

   c_set_init( &info.set);
   info.fp = fp;

   fprintf( fp, "digraph mulle_objc_class\n{\n");

   mulle_objc_runtime_class_walk( cls->runtime, cls, mulle_objc_runtime_is_class, callback, NULL, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


void   mulle_objc_class_dump_graphviz_to_file( struct _mulle_objc_class *cls, char *filename)
{
   FILE   *fp;

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_class_dump_graphviz( cls, fp);
   fclose( fp);

   fprintf( stderr, "Dumped \"%s\"\n", filename);
}


void   mulle_objc_class_dump_graphviz_tmp( struct _mulle_objc_class *cls)
{
   mulle_objc_class_dump_graphviz_to_file( cls, "/tmp/mulle_objc_class.dot");
}


