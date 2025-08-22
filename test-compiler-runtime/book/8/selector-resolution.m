// Test for Chapter 8: Selector Resolution
// Tests string to selector ID mapping

#import "include.h"

@interface TestClass
- (const char *)testMethod;
+ (int)classMethod;
@end

@implementation TestClass
- (const char *)testMethod { return "instance"; }
+ (int)classMethod { return 42; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    mulle_objc_methodid_t selector_id;
    const char *method_name;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("Test: Selector resolution from string to ID and back\n");
    
    // Test instance method selector
    selector_id = mulle_objc_methodid_from_string("testMethod");
    mulle_printf("Selector ID for 'testMethod': %llu\n", (unsigned long long) selector_id);
    
    method_name = mulle_objc_universe_lookup_methodname(universe, selector_id);
    mulle_printf("Method name for ID: %s\n", method_name ? method_name : "NULL");
    
    // Test class method selector
    selector_id = mulle_objc_methodid_from_string("classMethod");
    mulle_printf("Selector ID for 'classMethod': %llu\n", (unsigned long long) selector_id);
    
    method_name = mulle_objc_universe_lookup_methodname(universe, selector_id);
    mulle_printf("Method name for ID: %s\n", method_name ? method_name : "NULL");
    
    // Test non-existent selector
    selector_id = mulle_objc_methodid_from_string("nonexistentMethod");
    mulle_printf("Selector ID for 'nonexistentMethod': %llu\n", (unsigned long long) selector_id);
    
    method_name = mulle_objc_universe_lookup_methodname(universe, selector_id);
    mulle_printf("Method name for ID: %s\n", method_name ? method_name : "NULL");
    
    return 0;
}