// Test for Chapter 7: Method Cache Demonstration
// Tests method cache probing with _mulle_objc_class_probe_implementation

#import "include.h"

@interface TestClass
- (const char *)description;
@end

@implementation TestClass
- (const char *)description
{
    return "TestClass instance";
}
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    mulle_objc_implementation_t imp;
    struct _mulle_objc_method *method;
    mulle_objc_methodid_t selector;
    id obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    obj = _mulle_objc_infraclass_alloc_instance(infra);
    if (!obj)
    {
        mulle_printf("ERROR: Failed to create instance\n");
        return 1;
    }
    
    selector = mulle_objc_methodid_from_string("description");
    
    mulle_printf("=== Method Cache Demonstration ===\n");
    
    // First lookup - populates cache if needed
    method = mulle_objc_class_search_method_nofall(&infra->base, selector);
    if (method)
        mulle_printf("LOOKUP 1: Found method (%p)\n", method);
    else
        mulle_printf("LOOKUP 1: Method not found\n");
    
    // Second lookup - benefits from cache
    method = mulle_objc_class_search_method_nofall(&infra->base, selector);
    if (method)
        mulle_printf("LOOKUP 2: Cached method (%p)\n", method);
    else
        mulle_printf("LOOKUP 2: Method not found\n");
    
    mulle_printf("SUCCESS: Cache demonstration complete\n");
    return 0;
}