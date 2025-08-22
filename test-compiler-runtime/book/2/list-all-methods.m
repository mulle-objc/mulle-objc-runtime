// Test for Chapter 2: List All Methods
// Tests listing all methods of a class

#import "include.h"
#include <stdio.h>

@interface TestClass
- (void)instanceMethod;
+ (void)classMethod;
@end
@implementation TestClass
- (void)instanceMethod {}
+ (void)classMethod {}
@end

static mulle_objc_walkcommand_t print_method_name(struct _mulle_objc_method *method,
                                                  struct _mulle_objc_methodlist *list,
                                                  struct _mulle_objc_class *cls,
                                                  void *userinfo)
{
    printf("  %s\n", _mulle_objc_method_get_name(method));
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    printf("Methods in TestClass:\n");
    _mulle_objc_class_walk_methods(&cls->base, 0, print_method_name, NULL);
    
    return 0;
}