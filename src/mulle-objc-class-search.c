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
   __mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                     struct _mulle_objc_searcharguments *search,
                                     unsigned int inheritance,
                                     struct _mulle_objc_searchresult *result,
                                     enum internal_search_mode *mode);


#pragma mark - trace support

static void   trace_method_start( struct _mulle_objc_class *cls,
                                  struct _mulle_objc_searcharguments *search)
{
   struct _mulle_objc_universe   *universe;
   char                          buf[ s_mulle_objc_sprintf_functionpointer_buffer + 32];
   char                          *name;

   universe = _mulle_objc_class_get_universe( cls);
   if( search->args.mode == search_imp)
   {
      mulle_objc_universe_trace_nolf( universe,
                                      "start search for "
                                      "imp %p in %s %08x \"%s\"",
                                      search->imp,
                                      _mulle_objc_class_get_classtypename( cls),
                                      _mulle_objc_class_get_classid( cls),
                                      _mulle_objc_class_get_name( cls));
   }
   else
   {
      name = _mulle_objc_universe_describe_methodid( universe, search->args.methodid);
      mulle_objc_universe_trace_nolf( universe,
                                      "start search for "
                                      "methodid %08x \"%s\" in %s %08x \"%s\"",
                                      search->args.methodid,
                                      name,
                                      _mulle_objc_class_get_classtypename( cls),
                                      _mulle_objc_class_get_classid( cls),
                                      _mulle_objc_class_get_name( cls));
   }

   switch( search->args.mode)
   {
   case search_previous_method   :
      mulle_objc_sprintf_functionpointer( buf, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( search->previous_method));
      fprintf( stderr, " (previous method=%p (IMP=%p))\n",
              search->previous_method,
              buf);
      return;

   case search_specific_method :
      fprintf( stderr, " (specific=%08x,%08x)\n",
              search->args.classid,
              search->args.categoryid);
      return;

   case search_super_method :
      fprintf( stderr, " (super=%08x)\n",
              search->args.classid);
      return;

   case search_overridden_method :
      fprintf( stderr, " (overridden=%08x,%08x)\n",
              search->args.classid,
              search->args.categoryid);
      return;
   }
   fprintf( stderr, "\n");
}


static void   trace_method_done( struct _mulle_objc_class *cls,
                                 struct _mulle_objc_searcharguments *search,
                                 struct _mulle_objc_method *method)
{
   struct _mulle_objc_universe   *universe;
   char                          buf[ s_mulle_objc_sprintf_functionpointer_buffer + 32];

   universe = _mulle_objc_class_get_universe( cls);
   if( search->args.mode == search_imp)
   {
      mulle_objc_sprintf_functionpointer( buf, (mulle_functionpointer_t) method);
      mulle_objc_universe_trace( universe, "found method %p", buf);
   }
   else
   {
      mulle_objc_sprintf_functionpointer( buf, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method));
      mulle_objc_universe_trace( universe, "found method IMP %s", buf);
   }
}


static void   trace_method_search_fail( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_searcharguments *search,
                                        int error)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   switch( error)
   {
   case ENOENT:
      mulle_objc_universe_trace( universe, "method not found");
      break;

   case EINVAL:
      mulle_objc_universe_trace( universe, "invalid search call");
      break;

   default :
      mulle_objc_universe_trace( universe, "method search error %d", error);
   }
}


static void   trace_method_found( struct _mulle_objc_class *cls,
                                  struct _mulle_objc_searcharguments *search,
                                  struct _mulle_objc_methodlist *list,
                                  struct _mulle_objc_method *method,
                                  struct mulle_concurrent_pointerarrayreverseenumerator *rover)
{
   struct _mulle_objc_universe   *universe;
   char                          *categoryname;

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

   fprintf( stderr, " methodid %08x ( \"%s\")\"\n",
           method->descriptor.methodid,
           method->descriptor.name);
}


static void   trace_search( struct _mulle_objc_class *cls,
                            struct _mulle_objc_searcharguments *search,
                            unsigned int inheritance,
                            enum internal_search_mode mode)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   mulle_objc_universe_trace( universe,
                              "search %s %08x \"%s\" (0x%x) %p",
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

   found        = MULLE_OBJC_METHOD_SEARCH_FAIL;
   pair         = _mulle_objc_class_get_classpair( cls);
   infra        = _mulle_objc_classpair_get_infraclass( pair);
   is_meta      = _mulle_objc_class_is_metaclass( cls);
   inheritance |= MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS;

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

      method = __mulle_objc_class_search_method( walk_cls,
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

      found = method;

      if( ! _mulle_objc_descriptor_is_hidden_override_fatal( &method->descriptor))
         break;
   }
   _mulle_objc_protocolclassreverseenumerator_done( &rover);

   return( found);
}



//
// if we are in the metaclass root, we would wrap to the infraclass
// That we don't want IF there are protocolclasses. In case of
// protocolclasses, we instead would like to wrap to the infraclass of
// the protocolclass, after the meta paths through the protocolclasses
// have been exhausted
//
static struct _mulle_objc_class   *search_superclass( struct _mulle_objc_class *cls,
                                                      unsigned int inheritance)
{
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_protocolclassenumerator   rover;
   struct _mulle_objc_infraclass                *infra;
   struct _mulle_objc_metaclass                 *meta;
   struct _mulle_objc_class                     *supercls;
   struct _mulle_objc_class                     *protocls;

   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS)
      return( NULL);

   supercls =_mulle_objc_class_get_superclass( cls);
   if( _mulle_objc_class_is_infraclass( cls))
      return( supercls);
   if( _mulle_objc_class_is_metaclass( supercls))
      return( supercls);

   if( ! (inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META))
   {
      // Ok we'd be transitioning from metaclass to infraclass
      // Use protocolclass if available
      protocls = NULL;

      pair  = _mulle_objc_class_get_classpair( cls);
      rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);

      for(;;)
      {
         infra = _mulle_objc_protocolclassenumerator_next( &rover);
         if( ! infra)
            break;

         meta     = _mulle_objc_infraclass_get_metaclass( infra);
         protocls = _mulle_objc_metaclass_as_class( meta);
         if( protocls != cls)
         {
            supercls = protocls;
            break;
         }
      }
      _mulle_objc_protocolclassenumerator_done( &rover);
   }

   return( supercls);
}


//
// if previous is set, the search will be done for the method that previous
// has overriden.
// Returns NULL : error (and sets errno)
//         MULLE_OBJC_METHOD_SEARCH_FAIL for not found
//         method : when found
//
static struct _mulle_objc_method   *
   __mulle_objc_class_search_method( struct _mulle_objc_class *cls,
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
   unsigned int                                            n;
   unsigned int                                            tmp;


   // only enable first (@implementation of class) on demand
   //  ->[0]    : implementation
   //  ->[1]    : category
   //  ->[n -1] : last category

   found    = MULLE_OBJC_METHOD_SEARCH_FAIL;
   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_searches)
      trace_search( cls, search, inheritance, *mode);

   switch( *mode)
   {
   case search_super_method      :
      if( search->args.classid == cls->classid)
         *mode += OFFSET_2_MODE;
      goto next_class;

   case search_overridden_method :
   case search_specific_method   :
      if( search->args.classid != cls->classid)
         goto next_class;
      *mode += OFFSET_2_MODE;
      break;
   }

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
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

      if( ! _mulle_objc_descriptor_is_hidden_override_fatal( &method->descriptor))
      {
         // atomicity needed or not ? see header for more discussion
         method->descriptor.bits |= _mulle_objc_method_searched_and_found;

         if( universe->debug.trace.method_searches)
            trace_method_found( cls, search, list, method, &rover);

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

   //
   // searching the superclass for owner seems wanted
   //
   supercls = search_superclass( cls, inheritance);
   if( supercls)
   {
      method = __mulle_objc_class_search_method( supercls,
                                                search,
                                                supercls->inheritance,
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

   return( found);
}


# pragma mark API

//
// wrapper function, to not expose internal use of MULLE_OBJC_METHOD_SEARCH_FAIL
//
struct _mulle_objc_method   *
   mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                   struct _mulle_objc_searcharguments *search,
                                   unsigned int inheritance,
                                   struct _mulle_objc_searchresult *result)
{
   struct _mulle_objc_method                *method;
   enum internal_search_mode                mode;
   static struct _mulle_objc_searchresult   dummy;  // make static to reduce stack pain
   int                                      trace;

   if( ! result)
      result = &dummy;

   if( ! cls || ! search)
   {
      result->error = EINVAL;
      return( NULL);
   }

   //
   // The dange of the lookup code is that it can be used to bypass the
   // class initialization and call IMPs directly. So we need to be at least
   // in -initializing before we can actually search a method, otherwise
   // we get into problems.
   //
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_INITIALIZING))
      _mulle_objc_class_setup( cls);

   _mulle_objc_searcharguments_assert( search);

   assert( mulle_objc_class_is_current_thread_registered( cls));

   mode = (enum internal_search_mode) search->args.mode;
   switch( mode)
   {
   case search_default           :
   case search_imp               :
   case search_previous_method   :
   case search_specific_method   :
   case search_super_method      :
   case search_overridden_method :
      break;

   default :
      result->error = EINVAL;
      return( NULL);
   }

   trace = cls->universe->debug.trace.method_searches;
   if( trace)
      trace_method_start( cls, search);

   result->error = ENOENT;
   method = __mulle_objc_class_search_method( cls,
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

   if( trace)
      trace_method_done( cls, search, method);
   return( method);
}


struct _mulle_objc_method  *
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
   _mulle_objc_searcharguments_defaultinit( &search, methodid);
   method = mulle_objc_class_search_method( cls,
                                            &search,
                                            inheritance,
                                            &result);
   if( ! method)
      *error = result.error;
   return( method);
}


struct _mulle_objc_method  *
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

   _mulle_objc_searcharguments_defaultinit( &search, methodid);
   method = mulle_objc_class_search_method( cls,
                                            &search,
                                            inheritance,
                                            &result);
   if( ! method)
      *error = result.error;
   return( method);
}

# pragma mark - forwarding

static inline struct _mulle_objc_method   *
   _mulle_objc_class_search_forwardmethod( struct _mulle_objc_class *cls,
                                           int *error)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_defaultsearch_method( cls, MULLE_OBJC_FORWARD_METHODID, error);
   if( ! method)
      method = cls->universe->classdefaults.forwardmethod;

   return( method);
}


struct _mulle_objc_method    *
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


MULLE_C_NO_RETURN void
   _mulle_objc_class_fail_methodnotfound( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   mulle_objc_universe_fail_methodnotfound( universe, cls, missing_method);
}


MULLE_C_NO_RETURN void
   _mulle_objc_class_fail_fowardmethodnotfound( struct _mulle_objc_class *cls,
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
       mulle_objc_universe_fail_inconsistency( universe, "mulle_objc_universe: \"forward:\" method has wrong id in %sclass \"%s\"",
                                               prefix,
                                               name);
   if( missing_method)
      mulle_objc_universe_fail_methodnotfound( universe, cls, missing_method);

   mulle_objc_universe_fail_inconsistency( universe, "mulle_objc_universe: missing \"forward:\" method in %sclass \"%s\"",
                                                     prefix,
                                                     name);
}


MULLE_C_NONNULL_RETURN struct _mulle_objc_method *
   _mulle_objc_class_get_forwardmethod_lazy_nofail( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_method   *method;
   int                         error;

   method = _mulle_objc_class_lazyget_forwardmethod( cls, &error);
   if( method)
      return( method);

   _mulle_objc_class_fail_fowardmethodnotfound( cls, missing_method, error);
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


