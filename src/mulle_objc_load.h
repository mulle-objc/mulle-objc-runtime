//
//  mulle_objc_load.h
//  mulle-objc
//
//  Created by Nat! on 19.11.14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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

#ifndef mulle_objc_load_h__
#define mulle_objc_load_h__

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "mulle_objc_uniqueid.h"


struct _mulle_objc_category;
struct _mulle_objc_callqueue;
struct _mulle_objc_class;
struct _mulle_objc_infraclass;
struct _mulle_objc_ivarlist;
struct _mulle_objc_methodlist;
struct _mulle_objc_propertylist;
struct _mulle_objc_protocollist;
struct _mulle_objc_runtime;

struct _mulle_objc_dependency
{
   mulle_objc_classid_t      classid;
   mulle_objc_categoryid_t   categoryid;
};



//
// up the number if binary loads are incompatible
// this is read and checked against by the compiler
//
#define MULLE_OBJC_RUNTIME_LOAD_VERSION   8


struct _mulle_objc_loadclass
{
   mulle_objc_classid_t              classid;
   char                              *classname;
   mulle_objc_hash_t                 classivarhash;

   mulle_objc_classid_t              superclassid;
   char                              *superclassname;
   mulle_objc_hash_t                 superclassivarhash;

   int                               fastclassindex;
   int                               instancesize;

   struct _mulle_objc_ivarlist       *instancevariables;

   struct _mulle_objc_methodlist     *classmethods;
   struct _mulle_objc_methodlist     *instancemethods;
   struct _mulle_objc_propertylist   *properties;

   struct _mulle_objc_protocollist   *protocols;
   mulle_objc_classid_t              *protocolclassids;

   char                              *origin;
};


struct _mulle_objc_loadcategory
{
   mulle_objc_categoryid_t           categoryid;
   char                              *categoryname;

   mulle_objc_classid_t              classid;
   char                              *classname;           // useful ??
   mulle_objc_hash_t                 classivarhash;

   struct _mulle_objc_methodlist     *classmethods;       // contains categoryid
   struct _mulle_objc_methodlist     *instancemethods;
   struct _mulle_objc_propertylist   *properties;

   struct _mulle_objc_protocollist   *protocols;
   mulle_objc_classid_t              *protocolclassids;

   char                              *origin;
};


struct _mulle_objc_loadclasslist
{
   unsigned int                    n_loadclasses;
   struct _mulle_objc_loadclass    *loadclasses[ 1];
};


static inline size_t  mulle_objc_sizeof_loadclasslist( unsigned int n_loadclasses)
{
   return( sizeof( struct _mulle_objc_loadclasslist) + (n_loadclasses - 1) * sizeof( struct _mulle_objc_loadclass *));
}


struct _mulle_objc_loadcategorylist
{
   unsigned int                      n_loadcategories;
   struct _mulle_objc_loadcategory   *loadcategories[ 1];
};


static inline size_t  mulle_objc_sizeof_loadcategorylist( unsigned int n_load_categories)
{
   return( sizeof( struct _mulle_objc_loadcategorylist) + (n_load_categories - 1) * sizeof( struct _mulle_objc_loadcategory *));
}


struct _mulle_objc_loadstringlist
{
   unsigned int                n_loadstrings;
   struct _mulle_objc_object   *loadstrings[ 1];
};


struct _mulle_objc_loadhashedstring
{
   mulle_objc_uniqueid_t   uniqueid;
   char                    *string;
};


struct _mulle_objc_loadhashedstringlist
{
   unsigned int                          n_loadentries;
   struct _mulle_objc_loadhashedstring   loadentries[ 1];
};


int   _mulle_objc_loadhashedstring_compare( struct _mulle_objc_loadhashedstring *a,
                                            struct _mulle_objc_loadhashedstring *b);

void   mulle_objc_loadhashedstring_sort( struct _mulle_objc_loadhashedstring *methods,
                                         unsigned int n);


char   *_mulle_objc_loadhashedstring_bsearch( struct _mulle_objc_loadhashedstring *buf,
                                              unsigned int n,
                                              mulle_objc_ivarid_t search);

static inline char   *mulle_objc_loadhashedstringlist_bsearch( struct _mulle_objc_loadhashedstringlist *map,
                                                          mulle_objc_ivarid_t search)
{
   if( map)
      return( _mulle_objc_loadhashedstring_bsearch( map->loadentries, map->n_loadentries, search));
   return( NULL);
}


static inline void   mulle_objc_loadhashedstringlist_sort( struct _mulle_objc_loadhashedstringlist *map)
{
   if( map)
      mulle_objc_loadhashedstring_sort( map->loadentries, map->n_loadentries);
}


//
// this adds version info to the loaded classes and the
// categories. .
//
enum _mulle_objc_loadinfobits
{
   _mulle_objc_loadinfo_unsorted      = 0x1,
   _mulle_objc_loadinfo_aaomode       = 0x2,
   _mulle_objc_loadinfo_notaggedptrs  = 0x4,
   _mulle_objc_loadinfo_threadlocalrt = 0x8,

   _mulle_objc_loadinfo_optlevel_0   = (0 << 8),  // actual values...
   _mulle_objc_loadinfo_optlevel_1   = (1 << 8),
   _mulle_objc_loadinfo_optlevel_2   = (2 << 8),
   _mulle_objc_loadinfo_optlevel_3   = (3 << 8),
   _mulle_objc_loadinfo_optlevel_s   = (7 << 8)

   // lower 16 bits for runtime

   // next 12 bits free for foundation (future: somehow)
   // last  4 bits free for user       (future: somehow)
};


//
// the load is the MULLE_OBJC_RUNTIME_LOAD_VERSION built into the compiler
// The runtime is the MULLE_OBJC_RUNTIME_VERSION read by the compiler.
// The foundation version is necessary for the fastcalls. Since the foundation
// defines the fastcalls and fastclasses, the foundation must match.
// Whenever the fastcall/fastclasses change you need to update the foundation
// number.
// There is no minor/major scheme with respect to foundation.
//
struct mulle_objc_loadversion
{
   uint32_t   load;
   uint32_t   runtime;
   uint32_t   foundation;
   uint32_t   user;
   uint32_t   bits;
};


struct _mulle_objc_loadinfo
{
   struct mulle_objc_loadversion             version;

   struct _mulle_objc_loadclasslist          *loadclasslist;
   struct _mulle_objc_loadcategorylist       *loadcategorylist;
   struct _mulle_objc_loadstringlist         *loadstringlist;
   struct _mulle_objc_loadhashedstringlist   *loadhashedstringlist;  // optional for debugging

   // v5 could have this ?
   //    char   *originator;        // can be nil, compiler writes __FILE__ here when executing in -O0
};


// should give the file that was used to compile it
char  *mulle_objc_loadinfo_get_originator( struct _mulle_objc_loadinfo *info);

# pragma mark  - "master" load call

//
// use this if the compiler was able to sort all protocol_uniqueids
// all method lists referenced by load_categories and loadclasses by their methodid
//
void   mulle_objc_loadinfo_unfailing_enqueue( struct _mulle_objc_loadinfo *info);

// checks that loadinfo is compatibly compiled
void    mulle_objc_runtime_assert_loadinfo( struct _mulle_objc_runtime *runtime,
                                           struct _mulle_objc_loadinfo *info);

# pragma mark - class

//
// use this function to determine, if the runtime is ready to load this class
// yet. Returns the class, that's not yet loaded.
//
void   mulle_objc_loadclass_unfailing_enqueue( struct _mulle_objc_loadclass *info,
                                               struct _mulle_objc_callqueue *loads);

void   mulle_objc_loadclass_print_unfulfilled_dependency( struct _mulle_objc_loadclass *info,
                                                          struct _mulle_objc_runtime *runtime);


# pragma mark - category

// same for categories
int   mulle_objc_loadcategory_is_categorycomplete( struct _mulle_objc_loadcategory *info,
                                                   struct _mulle_objc_infraclass *infra);

int    mulle_objc_loadcategory_enqueue( struct _mulle_objc_loadcategory *info,
                                        struct _mulle_objc_callqueue *loads);
void   mulle_objc_loadcategory_unfailing_enqueue( struct _mulle_objc_loadcategory *info,
                                                  struct _mulle_objc_callqueue *loads);

void   mulle_objc_loadcategory_print_unfulfilled_dependency( struct _mulle_objc_loadcategory *info,
                                                             struct _mulle_objc_runtime *runtime);

#endif
