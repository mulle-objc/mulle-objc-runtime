//
//  mulle_objc_class_struct.h
//  mulle-objc-runtime
//
//  Created by Nat! on 07.04.17.
//  Copyright © 2017 Mulle kybernetiK. All rights reserved.
//

#ifndef mulle_objc_class_struct_h__
#define mulle_objc_class_struct_h__


// minimal include to get the struct up

#include "mulle_objc_atomicpointer.h"
#include "mulle_objc_cache.h"
#include "mulle_objc_fastmethodtable.h"
#include "mulle_objc_kvccache.h"
#include "mulle_objc_objectheader.h"
#include "mulle_objc_uniqueid.h"

#include <mulle_concurrent/mulle_concurrent.h>


struct _mulle_objc_class;
struct _mulle_objc_callqueue;
struct _mulle_objc_ivarlist;
struct _mulle_objc_methodlist;
struct _mulle_objc_object;
struct _mulle_objc_propertylist;
struct _mulle_objc_runtime;


//
// the order of these structure elements is architecture dependent
// the trick is, that the mask is "behind" the buckets in malloced
// space. the vtab is in there, because runtime and class share this
// (why not make the runtime a first class object, because then
// the access to classes slows. We could put the runtime into another object
//

struct _mulle_objc_methodcachepivot
{
   struct _mulle_objc_cachepivot       pivot; // for atomic XCHG with pointer indirection
   mulle_objc_methodimplementation_t   call2;
};


#define MULLE_OBJC_ANY_OWNER  ((void *) -1)

enum
{
   MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS          = 0x01,
   MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES          = 0x02,
   MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS           = 0x04,
   MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES = 0x08
};

#define MULLE_OBJC_CLASS_S_FAST_METHODS  16


enum
{
   MULLE_OBJC_CACHE_INITIALIZED  = 0x1,
   MULLE_OBJC_ALWAYS_EMPTY_CACHE = 0x2
};


//
// this is a fairly hefty struct, and the runtime needs two for each class
// assume that each new class costs you 1K
//
struct _mulle_objc_class
{
   struct _mulle_objc_methodcachepivot    cachepivot;  // DON'T MOVE

   void                                   *(*call)( void *, mulle_objc_methodid_t, void *, struct _mulle_objc_class *);

   /* ^^^ keep above like this, or change mulle_objc_fastmethodtable fault */

   // keep runtime here for easier access
   // keep name here for debugging

   char                                    *name;            // offset (void **)[ 3]
   struct _mulle_objc_runtime              *runtime;

   struct _mulle_objc_class                *superclass;
   struct _mulle_objc_infraclass           *infraclass;      // meta calls (+) use this

   uintptr_t                               allocationsize;   // instancesize + header
   uintptr_t                               extensionoffset;  // 4 later #1#
   mulle_objc_classid_t                    classid;
   mulle_objc_classid_t                    superclassid;

   mulle_atomic_pointer_t                  thread;          // protects the empty cache
   mulle_atomic_pointer_t                  state;

   struct _mulle_objc_method               *forwardmethod;

   // general storage mechanism for class variable, only in the infraclass
   struct _mulle_objc_kvccachepivot        kvc;   // ? needed in meta

   uint16_t                                inheritance;
   uint16_t                                preloads;

   struct mulle_concurrent_pointerarray    methodlists;
   struct _mulle_objc_fastmethodtable      vtab;  // dont' move it up, debugs nicer here
};



# pragma mark - petty accessors


static inline struct _mulle_objc_class   *_mulle_objc_class_get_superclass( struct _mulle_objc_class *cls)
{
   return( cls->superclass);
}


static inline mulle_objc_classid_t   _mulle_objc_class_get_classid( struct _mulle_objc_class *cls)
{
   return( cls->classid);
}


static inline char   *_mulle_objc_class_get_name( struct _mulle_objc_class *cls)
{
   return( cls->name);
}


static inline struct _mulle_objc_runtime   *_mulle_objc_class_get_runtime( struct _mulle_objc_class *cls)
{
   return( cls->runtime);
}


static inline size_t   _mulle_objc_class_get_allocationsize( struct _mulle_objc_class *cls)
{
   return( cls->allocationsize);
}


static inline size_t   _mulle_objc_class_get_instancesize( struct _mulle_objc_class *cls)
{
   return( cls->allocationsize - sizeof( struct _mulle_objc_objectheader));
}


// deprecated
static inline size_t   _mulle_objc_class_get_instance_size( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_class_get_instancesize( cls));
}


static inline unsigned int   _mulle_objc_class_get_inheritance( struct _mulle_objc_class *cls)
{
   return( cls->inheritance);
}

static inline void   _mulle_objc_class_set_inheritance( struct _mulle_objc_class *cls, unsigned int inheritance)
{
   assert( (unsigned short) inheritance == inheritance);

   cls->inheritance = (unsigned short) inheritance;
}


int   _mulle_objc_class_set_state_bit( struct _mulle_objc_class *cls, unsigned int bit);

static inline unsigned int   _mulle_objc_class_get_state_bit( struct _mulle_objc_class *cls,
                                                              unsigned int bit)
{
   void   *old;

   old = _mulle_atomic_pointer_read( &cls->state);
   return( (unsigned int) (uintptr_t) old & bit);
}


static inline struct _mulle_objc_method   *_mulle_objc_class_get_forwardmethod( struct _mulle_objc_class *cls)
{
   return( cls->forwardmethod);
}


static inline void   _mulle_objc_class_set_forwardmethod( struct _mulle_objc_class *cls, struct _mulle_objc_method *method)
{
   assert( ! method || (method->descriptor.methodid != MULLE_OBJC_NO_METHODID &&
                        method->descriptor.methodid != MULLE_OBJC_INVALID_METHODID));

   cls->forwardmethod = method;
}

//
// if we are a metaclass, the infraclass is the "actual" ''infraclass''
//
static inline struct _mulle_objc_infraclass   *_mulle_objc_class_get_infraclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass);
}

// only used during new_classpair
static inline void   _mulle_objc_class_set_infraclass( struct _mulle_objc_class *cls,  struct _mulle_objc_infraclass *infra)
{
   cls->infraclass = infra;
}


static inline int   _mulle_objc_class_is_infraclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass == NULL);
}


static inline int   _mulle_objc_class_is_metaclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass != NULL);
}


static inline struct _mulle_objc_metaclass   *_mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_objectheader   *header;

   header = _mulle_objc_object_get_objectheader( (struct _mulle_objc_object *) cls);
   return( (struct _mulle_objc_metaclass *) _mulle_objc_objectheader_get_isa( header));
}


static inline char   *_mulle_objc_class_get_classtypename( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_class_is_metaclass( cls) ? "metaclass" : "infraclass");
}


#endif /* mulle_objc_class_struct_h */
