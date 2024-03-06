//
//  mulle_objc_loadinfo.h
//  mulle-objc-runtime
//
//  Created by Nat! on 01.04.20
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  Copyright (c) 2020 Codeon GmbH.
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
#include "mulle-objc-loadinfo.h"

#include "mulle-objc-builtin.h"
#include "mulle-objc-callqueue.h"
#include "mulle-objc-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-load.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-propertylist.h"
#include "mulle-objc-protocollist.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-universe-class.h"

#include <stdio.h>

# pragma mark - dump routines


static void  dump_bits( unsigned int bits, FILE *fp)
{
   char   *delim;

   delim ="";
   if( bits & _mulle_objc_loadinfo_unsorted)
   {
      fprintf( fp, "unsorted");
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_aaomode)
   {
      fprintf( fp, "%s.aam", delim);
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_notaggedptrs)
   {
      fprintf( fp, "%s-fobjc-no-tps", delim);
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_nofastcalls)
   {
      fprintf( fp, "%s-fobjc-no-fcs", delim);
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_threadaffineobjects)
   {
      fprintf( fp, "%s-fobjc-tao", delim);
      delim=", ";
   }


   fprintf( fp, "%s-O%u", delim, (bits >> 8) & 0x7);
}


static void   print_version( char *prefix, uint32_t version, FILE *fp)
{
   fprintf( fp, "%s=%u.%u.%u", prefix,
            mulle_objc_version_get_major( version),
            mulle_objc_version_get_minor( version),
            mulle_objc_version_get_patch( version));
}


static void   loadprotocolclasses_dump( mulle_objc_protocolid_t *protocolclassids,
                                        char *prefix,
                                        struct _mulle_objc_protocollist *protocols,
                                        FILE *fp)

{
   mulle_objc_protocolid_t      protoid;
   struct _mulle_objc_protocol  *protocol;

   for(; *protocolclassids; ++protocolclassids)
   {
      protoid = *protocolclassids;

      protocol = NULL;
      if( protocols)
         protocol = _mulle_objc_protocollist_search( protocols, protoid);
      if( protocol)
         fprintf( fp, "%s@class %s;\n%s@protocol %s;\n",
                          prefix, protocol->name, prefix, protocol->name);
      else
         fprintf( fp, "%s@class %08lx;\n%s@protocol #%08lx;\n",
                          prefix, (unsigned long) protoid, prefix, (unsigned long) protoid);
   }
}


static void   loadprotocols_dump( struct _mulle_objc_protocollist *protocols,
                                  FILE *fp)

{
   struct _mulle_objc_protocol   *p;
   struct _mulle_objc_protocol   *sentinel;
   char                          *sep;

   fprintf( fp, " <");
   sep = " ";
   p        = protocols->protocols;
   sentinel = &p[ protocols->n_protocols];
   for(; p < sentinel; ++p)
   {
      fprintf( fp, "%s%s", sep, p->name);
      sep = ", ";
   }
   fprintf( fp, ">");
}


static void   loadmethod_dump( struct _mulle_objc_method *method,
                               char *prefix,
                               char type,
                               FILE *fp)
{
   fprintf( fp, "%s %c%s; // id=%08lx signature=\"%s\" bits=0x%lx\n",
                prefix,
                type,
                method->descriptor.name,
                (unsigned long) method->descriptor.methodid,
                method->descriptor.signature,
                (unsigned long) method->descriptor.bits);

}


static void   loadsuper_dump( struct _mulle_objc_super *p,
                              char *prefix,
                              struct _mulle_objc_loadhashedstringlist *strings,
                              struct _mulle_objc_universe *universe,
                              FILE *fp)
{
   char   *classname;
   char   *methodname;

   // because we aren't sorted necessarily use slow search
   classname  = mulle_objc_loadhashedstringlist_search( strings, p->classid);
   if( ! classname)
      classname = _mulle_objc_universe_describe_classid( universe, p->superid);
   methodname = mulle_objc_loadhashedstringlist_search( strings, p->methodid);
   if( ! methodname && universe)
      methodname = _mulle_objc_universe_describe_methodid( universe, p->superid);

   fprintf( fp, "%s // super %08lx \"%s\" is class %08lx \"%s\" "
                    "and method %08lx \"%s\"\n",
                prefix,
                (unsigned long) p->superid,
                p->name,
                (unsigned long)  p->classid, classname,
                (unsigned long)  p->methodid, methodname);
}


static void   loadivar_dump( struct _mulle_objc_ivar *ivar,
                             char *prefix,
                             FILE *fp)
{
   fprintf( fp, "%s    %s; // @%ld id=%08lx signature=\"%s\"\n",
                prefix,
                ivar->descriptor.name,
                (long) ivar->offset,
                (unsigned long) ivar->descriptor.ivarid,
                ivar->descriptor.signature);
}


static void   loadproperty_dump( struct _mulle_objc_property *property,
                                 char *prefix,
                                 FILE *fp)
{
   fprintf( fp, "%s @property %s; // id=%08lx ivarid=%08lx signature=\"%s\" get=%08lx set=%08lx bits=0x%lx\n",
                prefix,
                property->name,
                (unsigned long) property->propertyid,
                (unsigned long) property->ivarid,
                property->signature,
                (unsigned long) property->getter,
                (unsigned long) property->setter,
                (unsigned long) property->bits);
}


static void   loadclass_dump( struct _mulle_objc_loadclass *p,
                              char *prefix,
                              FILE *fp)
{
   if( p->protocolclassids)
      loadprotocolclasses_dump( p->protocolclassids, prefix, p->protocols, fp);

   fprintf( fp, "%s@implementation %s", prefix, p->classname);
   if( p->superclassname)
      fprintf( fp, " : %s", p->superclassname);

   if( p->protocols)
      loadprotocols_dump( p->protocols, fp);

   fprintf( fp, " // %08lx : %08lx (#%08lx : #%08lx) fci=%d isize=%d",
                                         (unsigned long) p->classid,
                                         (unsigned long) p->superclassid,
                                         (unsigned long) p->classivarhash,
                                         (unsigned long) p->superclassivarhash,
                                         p->fastclassindex,
                                         p->instancesize);
   if( p->origin)
      fprintf( fp, ", %s", p->origin);

   fprintf( fp, "\n");

   if( p->instancevariables)
   {
      fprintf( fp, "%s{\n", prefix);
      struct _mulle_objc_ivar   *ivar;
      struct _mulle_objc_ivar   *sentinel;

      ivar     = p->instancevariables->ivars;
      sentinel = &ivar[ p->instancevariables->n_ivars];
      while( ivar < sentinel)
      {
         loadivar_dump( ivar, prefix, fp);
         ++ivar;
      }
      fprintf( fp, "%s}\n", prefix);
   }

   if( p->properties)
   {
      struct _mulle_objc_property   *property;
      struct _mulle_objc_property   *sentinel;

      property = p->properties->properties;
      sentinel = &property[ p->properties->n_properties];
      while( property < sentinel)
      {
         loadproperty_dump( property, prefix, fp);
         ++property;
      }
   }

   if( p->classmethods)
   {
      struct _mulle_objc_method   *method;
      struct _mulle_objc_method   *sentinel;

      method   = p->classmethods->methods;
      sentinel = &method[ p->classmethods->n_methods];
      while( method < sentinel)
      {
         loadmethod_dump( method, prefix, '+', fp);
         ++method;
      }
   }

   if( p->instancemethods)
   {
      struct _mulle_objc_method   *method;
      struct _mulle_objc_method   *sentinel;

      method = p->instancemethods->methods;
      sentinel = &method[ p->instancemethods->n_methods];
      while( method < sentinel)
      {
         loadmethod_dump( method, prefix, '-', fp);
         ++method;
      }
   }

   fprintf( fp, "%s@end\n", prefix);
}


static void   loadcategory_dump( struct _mulle_objc_loadcategory *p,
                                 char *prefix,
                                 FILE *fp)
{
   struct _mulle_objc_method   *method;
   struct _mulle_objc_method   *sentinel;

   if( p->protocolclassids)
      loadprotocolclasses_dump( p->protocolclassids, prefix, p->protocols, fp);

   fprintf( fp, "%s@implementation %s( %s)", prefix, p->classname, p->categoryname);

   if( p->protocols)
      loadprotocols_dump( p->protocols, fp);

   fprintf( fp, " // %08lx,%08lx (#%08lx)", (unsigned long) p->classid,
                                            (unsigned long) p->categoryid,
                                            (unsigned long) p->classivarhash);
   if( p->origin)
      fprintf( fp, ", %s", p->origin);
   fprintf( fp, "\n");

   if( p->classmethods)
   {
      method = p->classmethods->methods;
      sentinel = &method[ p->classmethods->n_methods];
      while( method < sentinel)
      {
         loadmethod_dump( method, prefix, '+', fp);
         ++method;
      }
   }

   if( p->instancemethods)
   {
      method = p->instancemethods->methods;
      sentinel = &method[ p->instancemethods->n_methods];
      while( method < sentinel)
      {
         loadmethod_dump( method, prefix, '-', fp);
         ++method;
      }
   }

   fprintf( fp, "%s@end\n", prefix);
}



static void   loadclasslist_dump( struct _mulle_objc_loadclasslist *list,
                                  char *prefix,
                                  FILE *fp)
{
   struct _mulle_objc_loadclass   **p;
   struct _mulle_objc_loadclass   **sentinel;

   if( ! list)
      return;

   p        = list->loadclasses;
   sentinel = &p[ list->n_loadclasses];
   while( p < sentinel)
      loadclass_dump( *p++, prefix, fp);
}


static void   loadcategorylist_dump( struct _mulle_objc_loadcategorylist *list,
                                     char *prefix,
                                     FILE *fp)
{
   struct _mulle_objc_loadcategory   **p;
   struct _mulle_objc_loadcategory   **sentinel;

   if( ! list)
      return;

   p        = list->loadcategories;
   sentinel = &p[ list->n_loadcategories];
   while( p < sentinel)
      loadcategory_dump( *p++, prefix, fp);
}



static void   loadsuperlist_dump( struct _mulle_objc_superlist *list,
                                  char *prefix,
                                  struct _mulle_objc_loadhashedstringlist *strings,
                                  struct _mulle_objc_universe *universe,
                                  FILE *fp)
{
   struct _mulle_objc_super   *p;
   struct _mulle_objc_super   *sentinel;

   if( ! list)
      return;

   p        = list->supers;
   sentinel = &p[ list->n_supers];
   while( p < sentinel)
      loadsuper_dump( p++, prefix, strings, universe, fp);
}


void   mulle_objc_loadinfo_dump_fp( struct _mulle_objc_loadinfo *info,
                                    char *prefix,
                                    struct _mulle_objc_universe *universe,
                                    FILE *fp)
{
   char   *origin;

   fprintf( fp, "%s", prefix);

   origin = mulle_objc_loadinfo_get_origin( info);
   if( origin)
      fprintf( fp, "\"%s\" ", origin);

   print_version( "universe", info->version.runtime, fp);
   print_version( ", foundation", info->version.foundation, fp);
   print_version( ", user", info->version.user, fp);
   fprintf( fp, " (");
   dump_bits( info->version.bits, fp);
   fprintf( fp, ")\n");

   loadclasslist_dump( info->loadclasslist, prefix, fp);
   loadcategorylist_dump( info->loadcategorylist, prefix, fp);
   loadsuperlist_dump( info->loadsuperlist,
                       prefix,
                       info->loadhashedstringlist,
                       universe,
                       fp);
   fprintf( fp, "%s\n", prefix);
}


void   mulle_objc_loadinfo_dump( struct _mulle_objc_loadinfo *info,
                                 char *prefix,
                                 struct _mulle_objc_universe *universe)
{
   mulle_objc_loadinfo_dump_fp( info, prefix, universe, stderr);
}


