// Chapter 9A: Step 1 - Define simple class in loadinfo structure
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

// Generate this ID with: mulle-objc-uniqueid SimpleClass
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x12345678)

// Define the actual struct for our class
struct SimpleClass
{
    // Empty for now - just a simple class
};

// Use exact same pattern as demo1.c
typedef struct _gnu_mulle_objc_methodlist
{
   unsigned int                n_methods;
   void                        *owner;
   struct _mulle_objc_method   methods[];
} _gnu_mulle_objc_methodlist;

typedef struct _gnu_mulle_objc_loadclasslist
{
   unsigned int                    n_loadclasses;
   struct _mulle_objc_loadclass    *loadclasses[];
} _gnu_mulle_objc_loadclasslist;

// Empty method list for simple class
static struct _gnu_mulle_objc_methodlist  SimpleClass_instance_methodlist =
{
   0,
   NULL
};

// Simple class definition
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

   NULL,
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

static struct _mulle_objc_loadinfo  load_info =
{
   {
      MULLE_OBJC_RUNTIME_LOAD_VERSION,
      MULLE_OBJC_RUNTIME_VERSION,
      0,
      0,
      0
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

    printf("Chapter 9A: Step 1 - Simple class definition\n");

    cls = mulle_objc_global_lookup_infraclass_nofail(MULLE_OBJC_DEFAULTUNIVERSEID, ___SimpleClass_classid);
    if (!cls)
    {
        printf("ERROR: SimpleClass not found\n");
        return 1;
    }

    printf("SUCCESS: SimpleClass found\n");
    printf("Class name: %s\n", _mulle_objc_infraclass_get_name(cls));
    printf("Instance size: %zu bytes\n", _mulle_objc_infraclass_get_instancesize(cls));

    return 0;
}