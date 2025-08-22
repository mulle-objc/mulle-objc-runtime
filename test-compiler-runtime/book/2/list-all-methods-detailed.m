// Test for Chapter 2: List All Methods Detailed
// Tests listing all methods with signatures

#import "include.h"
#include <stdio.h>

@interface TestClass
- (int)instanceMethod:(const char *)text;
+ (double)classMethod:(int)value;
@end
@implementation TestClass
- (int)instanceMethod:(const char *)text { return 0; }
+ (double)classMethod:(int)value { return 0.0; }
@end

static mulle_objc_walkcommand_t print_method_details(struct _mulle_objc_method *method,
                                                     struct _mulle_objc_methodlist *list,
                                                     struct _mulle_objc_class *cls,
                                                     void *userinfo)
{
    printf("  %s %s\n", 
           _mulle_objc_method_get_name(method),
           _mulle_objc_method_get_signature(method));
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
    
    printf("Detailed methods in TestClass:\n");
    _mulle_objc_class_walk_methods(&cls->base, 0, print_method_details, NULL);
    
    return 0;
}