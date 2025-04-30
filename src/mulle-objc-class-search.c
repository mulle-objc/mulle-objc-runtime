//
//  mulle_objc_class_search.c
//  mulle-objc-runtime
//
//  Created by Nat! on 17/08/03.
//  Copyright (c) 2014-2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014-2017 Codeon GmbH.
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
#include "mulle-objc-class-search.h"

#include "mulle-objc-builtin.h"
#include "mulle-objc-class.h"
#include "mulle-objc-class-initialize.h"
#include "mulle-objc-class-struct.h"
#include "mulle-objc-universe-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-method.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-super.h"
#include "mulle-objc-universe.h"


#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "include-private.h"


#define OFFSET_2_MODE      100
#define OFFSET_3_MODE      200


enum internal_search_mode
{
   search_default             = MULLE_OBJC_SEARCH_DEFAULT,
   search_imp                 = MULLE_OBJC_SEARCH_IMP,

   search_overridden_method   = MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD,
   search_previous_method     = MULLE_OBJC_SEARCH_PREVIOUS_METHOD,
   search_specific_method     = MULLE_OBJC_SEARCH_SPECIFIC_METHOD,
   search_super_method        = MULLE_OBJC_SEARCH_SUPER_METHOD,

   search_overridden_method_2 = OFFSET_2_MODE + search_overridden_method,
   search_previous_method_2   = OFFSET_2_MODE + search_previous_method,
   search_specific_method_2   = OFFSET_2_MODE + search_specific_method,
   search_super_method_2      = OFFSET_2_MODE + search_super_method,

   search_overridden_method_3 = OFFSET_3_MODE + search_overridden_method,
   search_specific_method_3   = OFFSET_3_MODE + search_specific_method
};


static struct _mulle_objc_method   *
   _mulle_objc_class_search_method_internal( struct _mulle_objc_class *cls,
                                             struct _mulle_objc_searcharguments *search,
                                             unsigned int inheritance,
                                             struct _mulle_objc_searchresult *result,
                                             enum internal_search_mode *mode);


#pragma mark - trace support

static void   trace_method_start( struct _mulle_objc_class *cls,
                                  struct _mulle_objc_searcharguments *search,
                                  unsigned int inheritance)
{
   struct _mulle_objc_universe   *universe;
   char                          *name;
   mulle_objc_implementation_t   imp;
   char                          *sep;

   mulle_buffer_do( buffer)
   {
      universe = _mulle_objc_class_get_universe( cls);
      if( search->args.mode == search_imp)
      {
         mulle_buffer_sprintf( buffer, "start search for "
                                       "imp %p in %s %08lx \"%s\"",
                                       search->imp,
                                       _mulle_objc_class_get_classtypename( cls),
                                       (unsigned long) _mulle_objc_class_get_classid( cls),
                                       _mulle_objc_class_get_name( cls));
      }
      else
      {
         name = _mulle_objc_universe_describe_methodid( universe, search->args.methodid);
         mulle_buffer_sprintf( buffer, "start search for "
                                       "methodid %08x \"%s\" in %s %08lx \"%s\"",
                                       search->args.methodid,
                                       name,
                                       _mulle_objc_class_get_classtypename( cls),
                                       (unsigned long) _mulle_objc_class_get_classid( cls),
                                       _mulle_objc_class_get_name( cls));
      }

      switch( search->args.mode)
      {
      case search_previous_method   :
         imp = _mulle_objc_method_get_implementation( search->previous_method);
         mulle_buffer_sprintf( buffer, " (previous method=%p (IMP=",
                                       search->previous_method);
         mulle_buffer_sprintf_functionpointer( buffer, (mulle_functionpointer_t) imp);
         mulle_buffer_add_string( buffer, "))");
         break;

      case search_specific_method :
         mulle_buffer_sprintf( buffer, " (specific=%08lx,%08lx)",
                 (unsigned long) search->args.classid,
                 (unsigned long) search->args.categoryid);
         break;

      case search_super_method :
         mulle_buffer_sprintf( buffer, " (super=%08lx)",
                 (unsigned long) search->args.classid);
         break;

      case search_overridden_method :
         mulle_buffer_sprintf( buffer, " (overridden=%08lx,%08lx)",
                 (unsigned long) search->args.classid,
                 (unsigned long) search->args.categoryid);
         break;
      }

      mulle_buffer_sprintf( buffer, " in inherit(");

      sep = "";
      if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES))
      {
         mulle_buffer_sprintf( buffer, "%scategories", sep);
         sep=", ";
      }
      if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CLASS))
      {
         mulle_buffer_sprintf( buffer, "%sclass", sep);
         sep=", ";
      }
      if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES))
      {
         mulle_buffer_sprintf( buffer, "%sprotocol-categories", sep);
         sep=", ";
      }
      if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
      {
         mulle_buffer_sprintf( buffer, "%sprotocols", sep);
         sep=", ";
      }
      if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
      {
         mulle_buffer_sprintf( buffer, "%ssuperclass", sep);
         sep=", ";
      }
      if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META))
      {
         mulle_buffer_sprintf( buffer, "%sprotocol-meta", sep);
         sep=", ";
      }
      mulle_buffer_sprintf( buffer, ")");

      mulle_objc_universe_trace( universe, "%s", mulle_buffer_get_string( buffer));
   }
}


static void   trace_method_done( struct _mulle_objc_class *cls,
                                 struct _mulle_objc_searcharguments *search,
                                 struct _mulle_objc_method *method)
{
   struct _mulle_objc_universe   *universe;
   char                          *s;

   universe = _mulle_objc_class_get_universe( cls);
   if( search->args.mode != search_imp)
   {
      mulle_objc_universe_trace( universe, "found method %p", method);
      return;
   }

   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "found method IMP ");
      mulle_buffer_sprintf_functionpointer( buffer, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method));
      s = mulle_buffer_get_string( buffer);
      mulle_objc_universe_trace( universe, "%s", s);
   }
}


static void   trace_method_search_fail( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_searcharguments *search,
                                        int error)
{
   struct _mulle_objc_universe   *universe;

   MULLE_C_UNUSED( search);

   universe = _mulle_objc_class_get_universe( cls);
   if( error == ENOENT)
   {
      mulle_objc_universe_trace( universe, "method not found");
      return;
   }

   if( error == EINVAL)
   {
      mulle_objc_universe_trace( universe, "invalid search call");
      return;
   }

   mulle_objc_universe_trace( universe, "method search error %d", error);
}


static void   trace_method_found( struct _mulle_objc_class *cls,
                                  struct _mulle_objc_searcharguments *search,
                                  struct _mulle_objc_methodlist *list,
                                  struct _mulle_objc_method *method)
{
   struct _mulle_objc_universe   *universe;
   char                          *categoryname;

   MULLE_C_UNUSED( search);

   universe = _mulle_objc_class_get_universe( cls);
   mulle_objc_universe_trace_nolf( universe,
                                   "found in %s ",
                                   _mulle_objc_class_get_classtypename( cls));

   // it's a category ?
   categoryname = _mulle_objc_methodlist_get_categoryname( list);
   if( categoryname)
      fprintf( stderr, "\"%s( %s)\"", _mulle_objc_class_get_name( cls), categoryname);
   else
      fprintf( stderr, "\"%s\"", _mulle_objc_class_get_name( cls));

   fprintf( stderr, " methodid %08lx ( \"%s\")\"\n",
            (unsigned long) method->descriptor.methodid,
            method->descriptor.name);
}


static void   trace_search( struct _mulle_objc_class *cls,
                            struct _mulle_objc_searcharguments *search,
                            unsigned int inheritance,
                            enum internal_search_mode mode)
{
   struct _mulle_objc_universe   *universe;

   MULLE_C_UNUSED( search);
   MULLE_C_UNUSED( mode);

   universe = _mulle_objc_class_get_universe( cls);
   mulle_objc_universe_trace( universe,
                              "search %s %08x \"%s\" (0x%x) %p",
                              _mulle_objc_class_get_classtypename( cls),
                              cls->classid,
                              cls->name,
                              inheritance,
                              cls);
}


static void   trace_skip( struct _mulle_objc_class *cls,
                          struct _mulle_objc_searcharguments *search,
                          unsigned int inheritance,
                          enum internal_search_mode mode)
{
   struct _mulle_objc_universe   *universe;

   MULLE_C_UNUSED( search);
   MULLE_C_UNUSED( mode);

   universe = _mulle_objc_class_get_universe( cls);
   mulle_objc_universe_trace( universe,
                              "skipping %s %08x \"%s\" (0x%x) %p",
                              _mulle_objc_class_get_classtypename( cls),
                              cls->classid,
                              cls->name,
                              inheritance,
                              cls);
}



#pragma mark - method searching

#define MULLE_OBJC_METHOD_SEARCH_FAIL  ((struct _mulle_objc_method *) -1)

static struct _mulle_objc_method  *
   _mulle_objc_class_protocol_search_method( struct _mulle_objc_class *cls,
                                             struct _mulle_objc_searcharguments *search,
                                             unsigned int inheritance,
                                             struct _mulle_objc_searchresult *result,
                                             enum internal_search_mode *mode)
{
   struct _mulle_objc_classpair                        *pair;
   struct _mulle_objc_infraclass                       *infra;
   struct _mulle_objc_class                            *walk_cls;
   struct _mulle_objc_infraclass                       *proto_cls;
   struct _mulle_objc_infraclass                       *next_proto_cls;
   struct _mulle_objc_protocolclassreverseenumerator   rover;
   struct _mulle_objc_method                           *found;
   struct _mulle_objc_method                           *method;
   int                                                 is_meta;
   int                                                 is_proto;

   found        = MULLE_OBJC_METHOD_SEARCH_FAIL;
   pair         = _mulle_objc_class_get_classpair( cls);
   infra        = _mulle_objc_classpair_get_infraclass( pair);

   // TODO: if we is a protocolclass ourselves, and we inherited protocols
   //       we don't search them. Gotta figure out if the compiler should
   //       set proper inheritance though ?
   is_proto = _mulle_objc_class_is_protocolclass( _mulle_objc_infraclass_as_class( infra));
   if( is_proto)
      return( found);

   inheritance   |= MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS
                    | MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS_INHERITANCE
//                    | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META
                    | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS;
   is_meta        = _mulle_objc_class_is_metaclass( cls);

   rover          = _mulle_objc_classpair_reverseenumerate_protocolclasses( pair);
   next_proto_cls = _mulle_objc_protocolclassreverseenumerator_next( &rover);
   while( (proto_cls = next_proto_cls))
   {
      next_proto_cls = _mulle_objc_protocolclassreverseenumerator_next( &rover);
      if( proto_cls == infra)
         continue;

      walk_cls = _mulle_objc_infraclass_as_class( proto_cls);
      if( is_meta)
         walk_cls = _mulle_objc_metaclass_as_class( _mulle_objc_infraclass_get_metaclass( proto_cls));

      method = _mulle_objc_class_search_method_internal( walk_cls,
                                                         search,
                                                         inheritance | walk_cls->inheritance,
                                                         result,
                                                         mode);
      if( method == MULLE_OBJC_METHOD_SEARCH_FAIL)
         continue;

      if( ! method)
      {
         found = NULL;
         break;
      }

      if( found != MULLE_OBJC_METHOD_SEARCH_FAIL)
      {
         result->error = EEXIST;
         found = NULL;
         break;
      }

      if( search->callback)
      {
         assert( result);
         switch( (search->callback)( cls, search, inheritance, result))
         {
         case mulle_objc_walk_ok :
            continue;

         case mulle_objc_walk_done :
            break;

         case mulle_objc_walk_dont_descend :
            // inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS;
            continue;

         case mulle_objc_walk_cancel :
            result->error = ECANCELED;
            return( NULL);

         case mulle_objc_walk_error :
            result->error = errno;
            return( NULL);
         }
      }

      found = method;

      if( ! _mulle_objc_descriptor_is_hidden_override_fatal( &method->descriptor))
         break;
   }
   _mulle_objc_protocolclassreverseenumerator_done( &rover);

   return( found);
}


static struct _mulle_objc_method  *
   _mulle_objc_class_search_method_super( struct _mulle_objc_class *cls,
                                          struct _mulle_objc_searcharguments *search,
                                          unsigned int inheritance,
                                          struct _mulle_objc_searchresult *result,
                                          enum internal_search_mode *mode)
{
   unsigned int                superinheritance;
   struct _mulle_objc_method   *method;

   //
   // Usually we inherit what super wants, its actually quite dangerous
   // to override this in searches (better only do this in direct
   // NSObject subclasses, as the playground is known)
   //
   superinheritance = (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS_INHERITANCE)
                      ? inheritance
                      : cls->inheritance;

   method = _mulle_objc_class_search_method_internal( cls,
                                                      search,
                                                      superinheritance,
                                                      result,
                                                      mode);
   return( method);
}


//
// if previous is set, the search will be done for the method that previous
// has overriden.
// Returns NULL : error (and sets errno)
//         MULLE_OBJC_METHOD_SEARCH_FAIL for not found
//         method : when found
//
static struct _mulle_objc_method   *
   _mulle_objc_class_search_method_internal( struct _mulle_objc_class *cls,
                                             struct _mulle_objc_searcharguments *search,
                                             unsigned int inheritance,
                                             struct _mulle_objc_searchresult *result,
                                             enum internal_search_mode *mode)
{
   struct _mulle_objc_universe                             *universe;
   struct _mulle_objc_method                               *found;
   struct _mulle_objc_method                               *method;
   struct _mulle_objc_class                                *supercls;
   struct _mulle_objc_methodlist                           *list;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            i;
   unsigned int                                            n;
   unsigned int                                            tmp;

   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   found    = MULLE_OBJC_METHOD_SEARCH_FAIL;
   universe = _mulle_objc_class_get_universe( cls);

   switch( *mode)
   {
   case search_super_method      :
      if( search->args.classid == cls->classid)
         *mode += OFFSET_2_MODE;
      if( universe->debug.trace.method_search)
         trace_skip( cls, search, inheritance, *mode);
      goto next_class;

   case search_overridden_method :
   case search_specific_method   :
      if( search->args.classid != cls->classid)
      {
         if( universe->debug.trace.method_search)
            trace_skip( cls, search, inheritance, *mode);
         goto next_class;
      }
      *mode += OFFSET_2_MODE;
   default :
      break;
   }

   if( _mulle_objc_class_get_classid( cls) == search->stop_classid)
      return( NULL); // NULL means stop

   if( universe->debug.trace.method_search)
      trace_search( cls, search, inheritance, *mode);

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;  // this works: see how concurrent_pointerarray_reverseenumerate

   i     = 0;
   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);
   while( (list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover)))
   {
      if( (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CLASS) && ++i == n)
         break;

      switch( *mode)
      {
      case search_overridden_method_2 :
         if( search->args.categoryid == _mulle_objc_methodlist_get_categoryid( list))
            *mode += OFFSET_3_MODE - OFFSET_2_MODE;
         continue;

      case search_specific_method_2   :
         if( search->args.categoryid != _mulle_objc_methodlist_get_categoryid( list))
            continue;
         *mode += OFFSET_3_MODE - OFFSET_2_MODE;
      default :
         break;
      }

      // this returns NULL when not found!
      if( *mode == search_imp)
         method = _mulle_objc_methodlist_impsearch( list, search->imp);
      else
         method = _mulle_objc_methodlist_search( list, search->args.methodid);

      if( ! method)
      {
         if( *mode == search_specific_method_3)
            return( MULLE_OBJC_METHOD_SEARCH_FAIL);
         continue;
      }

      switch( *mode)
      {
      case search_overridden_method_3 :
         // as protocolclasses can appear multiple times, ensure that
         // we didn't hit "self" again
         if( search->args.classid == cls->classid &&
             search->args.categoryid == _mulle_objc_methodlist_get_categoryid( list))
            continue;
         break;

      case search_specific_method_3 :
         return( method);

      case search_previous_method :
         if( search->previous_method == method)
         {
            *mode = search_previous_method_2;
            continue;
         }

      default :
         break;
      }

      if( found != MULLE_OBJC_METHOD_SEARCH_FAIL)
      {
         result->error = EEXIST;
         return( NULL);
      }

      if( result)
      {
         result->class  = cls;
         result->list   = list;
         result->method = method;
      }

      if( search->callback)
      {
         assert( result);
         switch( (search->callback)( cls, search, inheritance, result))
         {
         case mulle_objc_walk_ok :
            continue;

         case mulle_objc_walk_done :
            break;

         case mulle_objc_walk_dont_descend :
            inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS;
            continue;

         case mulle_objc_walk_cancel :
            result->error = ECANCELED;
            return( NULL);

         case mulle_objc_walk_error :
            result->error = errno;
            return( NULL);
         }
      }

      if( ! _mulle_objc_descriptor_is_hidden_override_fatal( &method->descriptor))
      {
         // atomicity needed or not ? see header for more discussion
         method->descriptor.bits |= _mulle_objc_method_searched_and_found;

         if( universe->debug.trace.method_search)
            trace_method_found( cls, search, list, method);

         mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
         return( method);
      }

      found = method;
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);

   //
   // if we didn't find any starting point, it's not good for some modes
   //
   switch( *mode)
   {
   case search_overridden_method_2 :
   case search_specific_method_2   :
      result->error = EINVAL;
      return( NULL);

   default :
      break;
   }

next_class:
   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      tmp = 0;
      if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES)
         tmp |= MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES;

      //
      // A protocol could well have a category of the same name and it would
      // match, which would be unexpected or would it ? Probably not.
      // Generally: MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES is not
      // enabled, so it is a non-issue.
      //
      method = _mulle_objc_class_protocol_search_method( cls,
                                                         search,
                                                         tmp,
                                                         result,
                                                         mode);
      if( method != MULLE_OBJC_METHOD_SEARCH_FAIL)
      {
         if( ! method)
            return( NULL);

         if( found != MULLE_OBJC_METHOD_SEARCH_FAIL)
         {
            result->error = EEXIST;
            return( NULL);
         }

         if( ! _mulle_objc_descriptor_is_hidden_override_fatal( &method->descriptor))
            return( method);

         found = method;
      }
   }

   // We have:
   //  @interface A         -a {}; +a{};
   //  @interface B : A     -b {}; +b{};
   //  @interface C < X, Y> -c {}; +c{};
   //  @interface D : C     -d {}; +d{};
   //  @protocolclass X     -x {}; +x{};
   //  @protocolclass Y     -y {}; +y{};
   //
   // Invariably at this point we have searched the protocols < X, Y>.
   // now there is a divergence depending on:
   //
   //
   // |    cls       | supercls    | action
   // |--------------|-------------|------------
   // | A infra      | null        | stop
   // | A meta       | A infra     | search A infra
   // | B infra      | A infra     | search A infra
   // | B meta       | A meta      | search A meta
   // | C infra      | null        | stop
   // | C meta       | C infra     | search (metas) Y,X,C infra
   // | D infra      | C infra     | search C infra
   // | D meta       | C meta      | search D meta
   //
   supercls =_mulle_objc_class_get_superclass( cls);
   if( ! supercls)
      return( found);

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS))
   {
      if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META
          && _mulle_objc_class_is_metaclass( cls)
          && _mulle_objc_class_is_infraclass( supercls))
      {
         if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS_INHERITANCE))
            inheritance = supercls->inheritance
                          | MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS_INHERITANCE;
         inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS
                        | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META;
      }

      method = _mulle_objc_class_search_method_super( supercls,
                                                      search,
                                                      inheritance,
                                                      result,
                                                      mode);
      if( method != MULLE_OBJC_METHOD_SEARCH_FAIL)
      {
         if( ! method)
            return( NULL);

         if( found != MULLE_OBJC_METHOD_SEARCH_FAIL)
         {
            result->error = EEXIST;
            return( NULL);
         }

         if( ! _mulle_objc_descriptor_is_hidden_override_fatal( &method->descriptor))
            return( method);

         found = method;  // ???
      }
   }

   return( found);
}


# pragma mark API



static inline int   _mulle_objc_class_is_initializing_or_initialized( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_infraclass   *infra;

   infra = _mulle_objc_class_get_infraclass( cls);
   if( infra)
      cls = _mulle_objc_infraclass_as_class( infra);
   // that bit is never cleared
   return( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_INITIALIZING));
}


//
// wrapper function, to not expose internal use of MULLE_OBJC_METHOD_SEARCH_FAIL
//
struct _mulle_objc_method   *
   mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                   struct _mulle_objc_searcharguments *search,
                                   unsigned int inheritance,
                                   struct _mulle_objc_searchresult *result)
{
   struct _mulle_objc_method         *method;
   enum internal_search_mode         mode;
   struct _mulle_objc_searchresult   dummy; 
   int                               trace;

   if( ! result)
      result = &dummy;

   if( ! cls || ! search)
   {
      result->error = EINVAL;
      return( NULL);
   }

   _mulle_objc_searcharguments_assert( search);

   mode = (enum internal_search_mode) search->args.mode;

   //
   // The danger of the lookup code is that it can be used to bypass the
   // class initialization and call IMPs directly. So we need to be at least
   // in -initializing before we can actually search a method, otherwise
   // we get into problems. Unless we are looking for +load/+unload though.
   //
   if( search->initialize && ! _mulle_objc_class_is_initializing_or_initialized( cls))
   {
      switch( mode)
      {
      case search_default         :
      case search_previous_method :
         if( (search->args.methodid == MULLE_OBJC_UNLOAD_METHODID ||
              search->args.methodid == MULLE_OBJC_LOAD_METHODID))
         {
            break;
         }
         MULLE_C_FALLTHROUGH;

      default :
         _mulle_objc_class_setup( cls);
      }
   }


   assert( mulle_objc_class_is_current_thread_registered( cls));

   switch( mode)
   {
   case search_default           :
   case search_imp               :
   case search_previous_method   :
   case search_specific_method   :
   case search_super_method      :
   case search_overridden_method :
      result->error = ENOENT;
      break;

   default :
      result->error = EINVAL;
      return( NULL);
   }

   trace = cls->universe->debug.trace.method_search;
   if( trace)
      trace_method_start( cls, search, inheritance);

   method = _mulle_objc_class_search_method_internal( cls,
                                                      search,
                                                      inheritance,
                                                      result,
                                                      &mode);

   //
   // these are relative searches, if we are still
   // in the original mode, then the "search anchor"
   // has not been found, which is an error, as the class or
   // category is actually missing
   //
   switch( mode)
   {
   case search_previous_method   :
   case search_super_method      :
   case search_overridden_method :
      if( trace)
         trace_method_search_fail( cls, search, EINVAL);
      result->error = EINVAL;
      return( NULL);
   default :
       break;
   }

   // this can happen if hidden override detection is on

   if( method == MULLE_OBJC_METHOD_SEARCH_FAIL)
   {
      if( result->error == EEXIST)
         mulle_objc_universe_fail_inconsistency( cls->universe,
                              "class %08x \"%s\" hidden method override of %08x \"%s\"",
                              _mulle_objc_class_get_classid( cls),
                              _mulle_objc_class_get_name( cls),
                              search->args.methodid,
                              _mulle_objc_universe_describe_methodid( cls->universe,
                                                                      search->args.methodid));

      assert( result->error == ENOENT);
      if( trace)
         trace_method_search_fail( cls, search, ENOENT);

      return( NULL);
   }

   if( trace && method)
      trace_method_done( cls, search, method);
   return( method);
}



struct _mulle_objc_method *
   _mulle_objc_class_defaultsearch_method( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid,
                                           int *error)
{
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   unsigned int                         inheritance;
   struct _mulle_objc_method            *method;

   if( ! cls)
   {
      *error = EINVAL;
      return( NULL);
   }

   inheritance = _mulle_objc_class_get_inheritance( cls);
   search      = mulle_objc_searcharguments_make_default( methodid);
   method      = mulle_objc_class_search_method( cls,
                                                 &search,
                                                 inheritance,
                                                 &result);
   if( ! method)
      *error = result.error;
   return( method);
}


struct _mulle_objc_method *
   mulle_objc_class_search_non_inherited_method( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t methodid,
                                                 int *error)
{
   unsigned int                         inheritance;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   struct _mulle_objc_method            *method;

   if( ! cls || ! error)
      return( NULL);

   inheritance = _mulle_objc_class_get_inheritance( cls) |
                    MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS;

   search = mulle_objc_searcharguments_make_default( methodid);
   method = mulle_objc_class_search_method( cls,
                                            &search,
                                            inheritance,
                                            &result);
   if( ! method)
      *error = result.error;
   return( method);
}


void
   _mulle_objc_class_fail_methodnotfound( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   mulle_objc_universe_fail_methodnotfound( universe, cls, missing_method);
}


# pragma mark - forwarding

void
   _mulle_objc_class_fail_forwardmethodnotfound( struct _mulle_objc_class *cls,
                                                mulle_objc_methodid_t missing_method,
                                                int error)
{
   char                          *prefix;
   char                          *name;
   struct _mulle_objc_universe   *universe;

   prefix = _mulle_objc_class_is_metaclass( cls) ? "meta-" : "";
   name   = _mulle_objc_class_get_name( cls);

   universe = _mulle_objc_class_get_universe( cls);
   if( error != ENOENT)
       mulle_objc_universe_fail_inconsistency( universe,
                                               "mulle_objc_universe: \"forward:\" method has wrong id in %sclass \"%s\"",
                                               prefix,
                                               name);
   if( missing_method)
      mulle_objc_universe_fail_methodnotfound( universe, cls, missing_method);

   mulle_objc_universe_fail_inconsistency( universe,
                                           "mulle_objc_universe: missing \"forward:\" method in %sclass \"%s\"",
                                           prefix,
                                           name);
}


static inline struct _mulle_objc_method *
   _mulle_objc_class_search_forwardmethod( struct _mulle_objc_class *cls,
                                           int *error)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_defaultsearch_method( cls, MULLE_OBJC_FORWARD_METHODID, error);
   if( ! method)
      method = cls->universe->classdefaults.forwardmethod;

   return( method);
}


struct _mulle_objc_method *
   _mulle_objc_class_lazyget_forwardmethod( struct _mulle_objc_class *cls,
                                            int *error)
{
   struct _mulle_objc_method   *method;

   assert( mulle_objc_class_is_current_thread_registered( cls));

   method = _mulle_objc_class_get_forwardmethod( cls);
   if( ! method)
   {
      method = _mulle_objc_class_search_forwardmethod( cls, error);
      if( method)
         _mulle_objc_class_set_forwardmethod( cls, method);
   }
   return( method);
}


struct _mulle_objc_method *
   _mulle_objc_class_get_forwardmethod_lazy_nofail( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_method   *method;
   int                         error;

   method = _mulle_objc_class_lazyget_forwardmethod( cls, &error);
   if( method)
      return( method);

   _mulle_objc_class_fail_forwardmethodnotfound( cls, missing_method, error);
}


// used by the debugger
mulle_objc_implementation_t
   mulle_objc_class_get_forwardimplementation( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;
   int                         error;

   if( ! cls)
      return( 0);
   method = _mulle_objc_class_search_forwardmethod( cls, &error);
   if( method)
      return( _mulle_objc_method_get_implementation( method));
   return( 0);
}


// needed by MulleObject
struct _mulle_objc_method *
   _mulle_objc_class_supersearch_method( struct _mulle_objc_class *cls,
                                         mulle_objc_superid_t superid)
{
   struct _mulle_objc_method            *method;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_searcharguments   args;
   struct _mulle_objc_super             *p;

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   args     = mulle_objc_searcharguments_make_super( p->methodid, p->classid);
   method   = mulle_objc_class_search_method( cls,
                                              &args,
                                              cls->inheritance,
                                              NULL);
   return( method);
}


struct _mulle_objc_method *
   _mulle_objc_class_supersearch_method_nofail( struct _mulle_objc_class *cls,
                                                mulle_objc_superid_t superid)
{
   struct _mulle_objc_method            *method;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_searcharguments   args;
   struct _mulle_objc_super             *p;

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   args     = mulle_objc_searcharguments_make_super( p->methodid, p->classid);
   method   = mulle_objc_class_search_method( cls,
                                              &args,
                                              cls->inheritance,
                                              NULL);
   if( ! method)
      method = _mulle_objc_class_get_forwardmethod_lazy_nofail( cls, args.args.methodid);
   return( method);
}
