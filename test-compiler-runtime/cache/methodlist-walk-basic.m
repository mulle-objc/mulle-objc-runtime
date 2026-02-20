#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

@interface TestClass
+ (void) method1;
+ (void) method2;
@end

@interface TestClass (Category)
+ (void) categoryMethod;
@end

@implementation TestClass
+ (void) method1 { }
+ (void) method2 { }
@end

@implementation TestClass (Category)
+ (void) categoryMethod { }
@end

static mulle_objc_walkcommand_t test_callback(struct _mulle_objc_class *cls,
                                             struct _mulle_objc_methodlist *list,
                                             void *userinfo)
{
   int *count = (int*)userinfo;
   (*count)++;
   
   mulle_printf("Class: %s, Methods: %d\n",
          _mulle_objc_class_get_name(cls), 
          list ? list->n_methods : 0);
   
   return mulle_objc_walk_ok;
}

int main(void)
{
   struct _mulle_objc_infraclass *infra;
   struct _mulle_objc_metaclass *meta;
   int count = 0;
   
   mulle_printf("Testing mulle_objc_class_methodlist_walk\n");
   
   infra = mulle_objc_global_lookup_infraclass_nofail(
      MULLE_OBJC_DEFAULTUNIVERSEID,
      mulle_objc_classid_from_string("TestClass"));
   
   meta = _mulle_objc_infraclass_get_metaclass(infra);
   
   mulle_printf("Walking TestClass metaclass methodlists:\n");
   mulle_objc_class_methodlist_walk(_mulle_objc_metaclass_as_class(meta), 
                                   test_callback, 
                                   &count);
   
   mulle_printf("Total methodlists visited: %d\n", count);
   
   return 0;
}
