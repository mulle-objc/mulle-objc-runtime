//
//  mulle-objc-universe-fail.h
//  mulle-objc-runtime
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014-2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014-2018 Codeon GmbH.
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
#include "mulle-objc-universe-fail.h"

#include "include-private.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "mulle-objc-builtin.h"


// MEMO: mulle-stacktrace is complicated to compile and not that good
//       so its been removed from 0.20

# pragma mark - errors when no universe is present


MULLE_C_NO_RETURN void
   _mulle_objc_vprintf_abort( char *format, va_list args)
{
   vfprintf( stderr, format ? format : "???", args);
   fprintf( stderr, "\n");

   // could improve this with symbolification but, it's a) debug
   // only and b) you get better a one in the debugger
#if defined( DEBUG) && defined( MULLE_STACKTRACE_VERSION)
   mulle_stacktrace_once( stderr);
#endif
   abort();
}


MULLE_C_NO_RETURN MULLE_C_NEVER_INLINE void
   _mulle_objc_printf_abort( char *format, ...)
{
   va_list   args;

   va_start( args, format);
   _mulle_objc_vprintf_abort( format, args);
   va_end( args);

#if defined( DEBUG) && defined( MULLE_STACKTRACE_VERSION)
   mulle_stacktrace_once( stderr);
#endif
   abort();
}


char   *mulle_objc_known_name_for_uniqueid( mulle_objc_uniqueid_t uniqueid)
{
   switch( uniqueid)
   {
   case MULLE_OBJC_ALLOC_METHODID          : return( "alloc");
   case MULLE_OBJC_AUTORELEASE_METHODID    : return( "autorelease");
   case MULLE_OBJC_CLASS_METHODID          : return( "class");
   case MULLE_OBJC_COPY_METHODID           : return( "copy");
   case MULLE_OBJC_DEALLOC_METHODID        : return( "dealloc");
   case MULLE_OBJC_DEINITIALIZE_METHODID   : return( "deinitialize");
   case MULLE_OBJC_DEPENDENCIES_METHODID   : return( "dependencies");
   case MULLE_OBJC_FINALIZE_METHODID       : return( "finalize");
   case MULLE_OBJC_FORWARD_METHODID        : return( "forward:");
   case MULLE_OBJC_INITIALIZE_METHODID     : return( "initialize");
   case MULLE_OBJC_INIT_METHODID           : return( "init");
   case MULLE_OBJC_INSTANTIATE_METHODID    : return( "instantiate");
   case MULLE_OBJC_LOAD_METHODID           : return( "load");
   case MULLE_OBJC_MUTABLECOPY_METHODID    : return( "mutableCopy");
   case MULLE_OBJC_OBJECT_METHODID         : return( "object");
   case MULLE_OBJC_RELEASE_METHODID        : return( "release");
   case MULLE_OBJC_RETAIN_METHODID         : return( "retain");
   case MULLE_OBJC_RETAINCOUNT_METHODID    : return( "retainCount");
   case MULLE_OBJC_UNLOAD_METHODID         : return( "unload");
   case MULLE_OBJC_GENERIC_GETTER_METHODID : return( ":");
   case MULLE_OBJC_WILLFINALIZE_METHODID   : return( "willFinalize");

   case MULLE_OBJC_ADDOBJECT_METHODID       : return( "addObject:");
   case MULLE_OBJC_REMOVEOBJECT_METHODID    : return( "removeObject:");
   case MULLE_OBJC_WILLCHANGE_METHODID      : return( "willChange");
   case MULLE_OBJC_WILLREADRELATIONSHIP_METHODID   : return( "willReadRelationship:");

   case MULLE_OBJC_MULLE_ALLOCATOR_METHODID : return( "mulleAllocator");

   case 0x47a9beb6                          : return( "MulleObjCLoader");
   case 0x58bb178a                          : return( "NSAutoreleasePool");
   case 0xa41284db                          : return( "NSException");
   }
   return( NULL);
}


MULLE_C_NO_RETURN static void
   _mulle_objc_abort_classnotfound( struct _mulle_objc_universe *universe,
                                      mulle_objc_classid_t missing_classid)
{
   _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: unknown class %08x \"%s\"",
                              universe,
                              missing_classid,
                              universe ? _mulle_objc_universe_describe_classid( universe, missing_classid) : "???");
}


MULLE_C_NO_RETURN static void
   _mulle_objc_abort_methodnotfound( struct _mulle_objc_universe *universe,
                                     struct _mulle_objc_class *cls,
                                     mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_descriptor  *desc;
   char   *methodname;
   char   *name;

   name       = cls ? _mulle_objc_class_get_name( cls) : "???";
   methodname = NULL;

   if( ! methodname && universe)
   {
      desc = _mulle_objc_universe_lookup_descriptor( universe, missing_method);
      if( desc)
         methodname = desc->name;
      else
         methodname = _mulle_objc_universe_search_hashstring( universe, missing_method);
   }

   if( ! methodname)
      methodname = mulle_objc_known_name_for_uniqueid( missing_method);

   // keep often seen output more user friendly
   if( ! methodname)
      _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: unknown %s "
                                "method %08x in class %08x \"%s\"",
                                universe,
                                mulle_objc_class_is_metaclass( cls) ? "class" : "instance",
                                missing_method,
                                mulle_objc_class_get_classid( cls),
                                name);

   _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: unknown method "
                             "%08x \"%c%s\" in class %08x \"%s\"",
                             universe,
                             missing_method,
                             mulle_objc_class_is_metaclass( cls) ? '+' : '-',
                             methodname,
                             mulle_objc_class_get_classid( cls),
                             name);
}


MULLE_C_NO_RETURN static void
   _mulle_objc_abort_supernotfound( struct _mulle_objc_universe *universe,
                                    mulle_objc_superid_t missing_superid)
{
   _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: "
                             "missing super %08x (needs to be registered first)",
                             universe,
                             missing_superid);
}


#pragma mark - vectoring failures

void   _mulle_objc_universe_init_fail( struct _mulle_objc_universe  *universe)
{
   universe->failures.fail           = _mulle_objc_vprintf_abort;
   universe->failures.inconsistency  = _mulle_objc_vprintf_abort;
   universe->failures.classnotfound  = _mulle_objc_abort_classnotfound;
   universe->failures.methodnotfound = _mulle_objc_abort_methodnotfound;
   universe->failures.supernotfound  = _mulle_objc_abort_supernotfound;
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_failv_generic( struct _mulle_objc_universe *universe,
                                       char *format,
                                       va_list args)
{
   if( ! universe || _mulle_objc_universe_is_uninitialized( universe))
      _mulle_objc_vprintf_abort( format, args);
   (*universe->failures.fail)( format, args);

   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_generic( struct _mulle_objc_universe *universe,
                                      char *format, ...)
{
   va_list   args;

   va_start( args, format);
   mulle_objc_universe_failv_generic( universe, format, args);
   va_end( args);
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_code(struct _mulle_objc_universe *universe, int errnocode)
{
   mulle_objc_universe_fail_generic( universe, "errno: %s (%d)", strerror(errnocode), errnocode);
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_perror(struct _mulle_objc_universe *universe, char *s)
{
   mulle_objc_universe_fail_generic( universe, "%s: %s (%d)",
                                                s ? s : "errno",
                                                strerror( errno),
                                                errno);
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_failv_inconsistency( struct _mulle_objc_universe *universe,
                                            char *format,
                                            va_list args)
{
   //
   // inconsistency ? even universe might not be setup properly. This is the only
   // exception doing this check
   //
   if( ! universe || ! universe->failures.inconsistency)
   {
      vfprintf( stderr, format, args);
      abort();;
   }
   (*universe->failures.inconsistency)( format, args);

   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_inconsistency( struct _mulle_objc_universe *universe,
                                            char *format, ...)
{
   va_list   args;

   va_start( args, format);
   mulle_objc_universe_failv_inconsistency( universe, format, args);
   // va_end( args);  // some sanitizers hate on this being there, some on not being there
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_supernotfound( struct _mulle_objc_universe *universe,
                                            mulle_objc_superid_t superid)
{
   if ( ! universe || _mulle_objc_universe_is_uninitialized(universe))
      _mulle_objc_abort_supernotfound( universe, superid);

   (*universe->failures.supernotfound)(universe, superid);
   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_classnotfound( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid)
{
   if( ! universe || _mulle_objc_universe_is_uninitialized( universe))
      _mulle_objc_abort_classnotfound( universe, classid);

   (*universe->failures.classnotfound)( universe, classid);
   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_methodnotfound( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_class *cls,
                                             mulle_objc_methodid_t methodid)
{
   if (! universe || _mulle_objc_universe_is_uninitialized(universe))
      _mulle_objc_abort_methodnotfound( universe, cls, methodid);

   (*universe->failures.methodnotfound)( universe, cls, methodid);
   abort();  // just make sure if fail returns
}


