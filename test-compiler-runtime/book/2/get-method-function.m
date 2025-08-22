// Test for Chapter 2: Get Method Function
// Tests getting method implementation pointer

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

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_method *method;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    method = mulle_objc_infraclass_defaultsearch_method(cls, mulle_objc_uniqueid_from_string("instanceMethod"));
    if (method) {
        printf("instanceMethod implementation: %p\n", _mulle_objc_method_get_implementation(method));
    }
    
    method = mulle_objc_infraclass_defaultsearch_method(cls, mulle_objc_uniqueid_from_string("classMethod"));
    if (method) {
        printf("classMethod implementation: %p\n", _mulle_objc_method_get_implementation(method));
    }
    
    return 0;
}