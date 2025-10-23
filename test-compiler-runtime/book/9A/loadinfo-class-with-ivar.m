// Chapter 9A: Step 2 - Add instance variable to the class
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

// Generated uniqueid for "SimpleClass"
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)

// Generated uniqueid for "value"
#define ___value___ivarid        MULLE_OBJC_IVARID( 0x40c292ce)

// Define the actual struct for our class with an ivar
struct SimpleClass
{
    int value;
};

// Use exact same pattern as demo1.c
typedef struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods;
   void                        *owner;
   struct _mulle_objc_method   methods[];
} _gnu_mulle_objc_methodlist;

typedef struct _gnu_mulle_objc_ivarlist
{
   unsigned int              n_ivars;
   struct _mulle_objc_ivar   ivars[];
} _gnu_mulle_objc_ivarlist;

typedef struct _gnu_mulle_objc_loadclasslist
{
   unsigned int                    n_loadclasses;
   struct _mulle_objc_loadclass    *loadclasses[];
} _gnu_mulle_objc_loadclasslist;

// Empty method list for now
static struct _gnu_mulle_objc_methodlist  SimpleClass_instance_methodlist =
{
   0,
   NULL
};

// Define the ivar list with one ivar
static struct _gnu_mulle_objc_ivarlist  SimpleClass_ivarlist =
{
   1,
   {
      {
         {
            ___value___ivarid,
            "value",
            "i"
         },
         offsetof(struct SimpleClass, value)
      }
   }
};

// Class definition with ivars
static struct _mulle_objc_loadclass  SimpleClass_loadclass =
{
   ___SimpleClass_classid,
   "SimpleClass",
   0,

   0,
   NULL,
   0,

   -1,
   sizeof(struct SimpleClass),

   (struct _mulle_objc_ivarlist *) &SimpleClass_ivarlist,
   NULL,
   (struct _mulle_objc_methodlist *) &SimpleClass_instance_methodlist,
   NULL,

   NULL
};

static struct _gnu_mulle_objc_loadclasslist  class_list =
{
   1,
   {
      &SimpleClass_loadclass
   }
};

#ifdef __MULLE_OBJC_NO_TPS__
# define TPS_BIT   0x4
#else
# define TPS_BIT   0
#endif

#ifdef __MULLE_OBJC_NO_FCS__
# define FCS_BIT   0x8
#else
# define FCS_BIT   0
#endif

#ifdef __MULLE_OBJC_TAO__
# define TAO_BIT   0x10
#else
# define TAO_BIT   0
#endif

static struct _mulle_objc_loadinfo  load_info =
{
   {
      MULLE_OBJC_RUNTIME_LOAD_VERSION,
      MULLE_OBJC_RUNTIME_VERSION,
      0,
      0,
      TPS_BIT | FCS_BIT | TAO_BIT
   },
   NULL,
   (struct _mulle_objc_loadclasslist *) &class_list,
   NULL,
   NULL,
   NULL,
   NULL
};

MULLE_C_CONSTRUCTOR(__load)
static void  __load()
{
   static int  has_loaded;
   if( has_loaded)
      return;
   has_loaded = 1;

   mulle_objc_loadinfo_enqueue_nofail( &load_info);
}

int main(void)
{
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_object *obj;
    struct _mulle_objc_ivar *ivar;

    printf("Chapter 9A: Step 2 - Class with instance variable\n");

    cls = mulle_objc_global_lookup_infraclass_nofail(MULLE_OBJC_DEFAULTUNIVERSEID, ___SimpleClass_classid);
    if (!cls)
    {
        printf("ERROR: SimpleClass not found\n");
        return 1;
    }

    printf("SUCCESS: SimpleClass found\n");
    printf("Class name: %s\n", _mulle_objc_infraclass_get_name(cls));
    printf("Instance size: %zu bytes\n", _mulle_objc_infraclass_get_instancesize(cls));

    // Look up the ivar
    ivar = _mulle_objc_infraclass_search_ivar(cls, ___value___ivarid);
    
    if (!ivar)
    {
        printf("ERROR: value ivar not found\n");
        return 1;
    }

    printf("SUCCESS: Found ivar '%s' with offset %d\n", 
           _mulle_objc_ivar_get_name(ivar),
           _mulle_objc_ivar_get_offset(ivar));

    return 0;
}