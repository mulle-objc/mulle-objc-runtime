// Test for Chapter 8: IMP Extraction
// Tests getting actual function pointer from method

#import "include.h"

@interface TestClass
- (int)getValue;
+ (const char *)getName;
@end

@implementation TestClass
- (int)getValue { return 42; }
+ (const char *)getName { return "TestClass"; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    struct _mulle_objc_method *method;
    void *imp;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: IMP extraction from methods\n");
    
    // Test instance method IMP extraction
    method = mulle_objc_infraclass_defaultsearch_method(infra, 
        mulle_objc_methodid_from_string("getValue"));
    
    if (method)
    {
        imp = _mulle_objc_method_get_implementation(method);
        mulle_printf("SUCCESS: Found getValue method\n");
        mulle_printf("IMP address: %p\n", imp);
    }
    else
    {
        mulle_printf("ERROR: getValue method not found\n");
    }
    
    // Test class method IMP extraction
    meta = mulle_objc_class_get_metaclass(&infra->base);
    method = mulle_objc_infraclass_defaultsearch_method(
        (struct _mulle_objc_infraclass *)meta,
        mulle_objc_methodid_from_string("getName"));
    
    if (method)
    {
        imp = _mulle_objc_method_get_implementation(method);
        mulle_printf("SUCCESS: Found getName class method\n");
        mulle_printf("IMP address: %p\n", imp);
    }
    else
    {
        mulle_printf("ERROR: getName class method not found\n");
    }
    
    return 0;
}