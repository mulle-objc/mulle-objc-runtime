// Test for Chapter 2: Get Class Struct
// Tests retrieving the raw class struct pointer

#import "include.h"
#include <stdio.h>

@interface TestClass
- (int)method;
@end
@implementation TestClass
- (int)method { return 42; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    printf("Class struct pointer: %p\n", (void *)cls);
    printf("Class name: %s\n", _mulle_objc_class_get_name(&cls->base));
    
    return 0;
}