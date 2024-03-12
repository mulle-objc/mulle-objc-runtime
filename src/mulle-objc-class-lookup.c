//
//  mulle-objc-class-lookup.c
//  mulle-objc-runtime
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
//  Copyright (c) 2021 Codeon GmbH.
//  All rights reserved.
//
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
#include "mulle-objc-class-lookup.h"

#include "include-private.h"

#include "mulle-objc-class-search.h"
#include "mulle-objc-impcache.h"
#include "mulle-objc-call.h"


# pragma mark -



mulle_objc_implementation_t
   _mulle_objc_class_probe_implementation( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_probe_implementation_inline( cls, methodid));
}


struct _mulle_objc_cacheentry *
   _mulle_objc_class_probe_cacheentry( struct _mulle_objc_class *cls,
                                       mulle_objc_superid_t methodid)
{
   return( _mulle_objc_class_probe_cacheentry_inline( cls, methodid));
}


enum
{
   mulle_objc_class_lookup_noprobe     = 0x1,   // unused
   mulle_objc_class_lookup_noforward   = 0x2,
   mulle_objc_class_lookup_lazyforward = 0x4,
   mulle_objc_class_lookup_nofail      = 0x8,
   mulle_objc_class_lookup_nofill      = 0x10   // don't write cache
};


//
// updates the cache, no forward
//
MULLE_C_ALWAYS_INLINE
static inline
   mulle_objc_implementation_t
      _mulle_objc_class_lookup_implementation_mode( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid,
                                                    unsigned int mode)
{
   struct _mulle_objc_method        *method;
   mulle_objc_implementation_t      imp;
   int                              error;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   if( ! (mode & mulle_objc_class_lookup_noprobe))
   {
      imp = _mulle_objc_class_probe_implementation( cls, methodid);
      if( imp)
      {
         if( mode & mulle_objc_class_lookup_noforward)
            if( _mulle_objc_class_is_forwardimplementation( cls, imp))
            {
               if( mode & mulle_objc_class_lookup_nofail)
                  _mulle_objc_class_fail_methodnotfound( cls, methodid);
               imp = 0;
            }
         return( imp);
      }
   }

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
   {
      if( mode & mulle_objc_class_lookup_lazyforward)
         method = _mulle_objc_class_lazyget_forwardmethod( cls, &error);
      if( ! method && (mode & mulle_objc_class_lookup_nofail))
         _mulle_objc_class_fail_fowardmethodnotfound( cls, methodid, error);
   }

   if( ! method)
   {
      if( mode & mulle_objc_class_lookup_nofail)
         _mulle_objc_class_fail_methodnotfound( cls, methodid);
      return( 0);
   }

   if( ! (mode & mulle_objc_class_lookup_nofill))
      _mulle_objc_class_fill_impcache( cls, NULL, method, methodid);

   imp = _mulle_objc_method_get_implementation( method);
   return( imp);
}


// fills the cache and does forward
mulle_objc_implementation_t
    _mulle_objc_class_lookup_implementation( struct _mulle_objc_class *cls,
                                             mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_lookup_implementation_mode( cls,
                                                         methodid,
                                                         mulle_objc_class_lookup_lazyforward));
}


//
// fills the cache and does forward, will raise if no method
// it knows about traces and the empty cache bit
//
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_lookup_implementation_mode( cls,
                                                         methodid,
                                                         mulle_objc_class_lookup_lazyforward
                                                         | mulle_objc_class_lookup_nofail));
}


mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofill( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_lookup_implementation_mode( cls,
                                                         methodid,
                                                         mulle_objc_class_lookup_lazyforward
                                                         | mulle_objc_class_lookup_nofill));
}


mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_noforward( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_lookup_implementation_mode( cls,
                                                         methodid,
                                                         mulle_objc_class_lookup_noforward));
}


// does not update the cache, no forward
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_noforward_nofill( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_lookup_implementation_mode( cls,
                                                         methodid,
                                                         mulle_objc_class_lookup_nofill
                                                         | mulle_objc_class_lookup_noforward));
}


mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail_nofill( struct _mulle_objc_class *cls,
                                                           mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_class_lookup_implementation_mode( cls,
                                                         methodid,
                                                         mulle_objc_class_lookup_nofill
                                                         | mulle_objc_class_lookup_lazyforward
                                                         | mulle_objc_class_lookup_nofail));
}



mulle_objc_implementation_t
   _mulle_objc_class_refresh_implementation_nofail( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;

   method = mulle_objc_class_search_method_nofail( cls, methodid);
   imp    = _mulle_objc_method_get_implementation( method);
   _mulle_objc_class_fill_impcache( cls, NULL, method, methodid);

   return( imp);
}



/*
 * super call
 */

//
// this is used for calling super. It's the same for metaclasses and
// infraclasses. The superid is hash( <classname> ';' <methodname>)
//
mulle_objc_implementation_t
   _mulle_objc_class_refresh_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                         mulle_objc_superid_t superid)
{
   struct _mulle_objc_method      *method;
   mulle_objc_implementation_t    imp;

   method = _mulle_objc_class_supersearch_method_nofail( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   _mulle_objc_class_fill_impcache( cls, NULL, method, superid);
   return( imp);
}



/* lookup cache first */

MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_class_search_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                                mulle_objc_superid_t superid)
{
   struct _mulle_objc_method             *method;
   struct _mulle_objc_universe           *universe;
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_super              *p;
   mulle_objc_implementation_t           imp;


   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   _mulle_objc_searcharguments_init_super( &args, p->methodid, p->classid);
   method = mulle_objc_class_search_method( cls,
                                            &args,
                                            cls->inheritance,
                                            NULL);
   if( ! method)
      method = _mulle_objc_class_get_forwardmethod_lazy_nofail( cls, args.args.methodid);

   imp = _mulle_objc_method_get_implementation( method);
   return( imp);
}


mulle_objc_implementation_t
   _mulle_objc_object_lookup_superimplementation_noforward_nofill( void *obj,
                                                                      mulle_objc_superid_t superid)
{
   struct _mulle_objc_class              *cls;
   struct _mulle_objc_method             *method;
   struct _mulle_objc_universe           *universe;
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_super              *p;
   mulle_objc_implementation_t           imp;


   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   cls      = _mulle_objc_object_get_isa( obj);
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   _mulle_objc_searcharguments_init_super( &args, p->methodid, p->classid);
   method = mulle_objc_class_search_method( cls,
                                            &args,
                                            cls->inheritance,
                                            NULL);
   if( ! method)
      return( 0);

   imp = _mulle_objc_method_get_implementation( method);
   return( imp);
}

//
// fills the cache
//
mulle_objc_implementation_t
   _mulle_objc_class_lookup_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid)
{
   return( _mulle_objc_class_lookup_superimplementation_inline_nofail( cls, superid));
}


mulle_objc_implementation_t
   _mulle_objc_object_lookup_superimplementation_nofail( void *obj,
                                                         mulle_objc_superid_t superid)
{
   return( _mulle_objc_object_lookup_superimplementation_inline_nofail( obj, superid));
}
