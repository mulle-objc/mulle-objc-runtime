// Test for Chapter 8: Method Lookup
// Tests hash table lookup algorithm for finding methods

#import "include.h"

@interface TestClass
- (const char *)description;
+ (int)computeValue;
@end

@implementation TestClass
- (const char *)description { return "TestClass instance"; }
+ (int)computeValue { return 123; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_method *method;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: Method lookup in hash table\n");
    
    // Test instance method lookup
    method = mulle_objc_infraclass_defaultsearch_method(infra, 
        mulle_objc_methodid_from_string("description"));
    
    if (method)
    {
        mulle_printf("SUCCESS: Found description method\n");
        mulle_printf("Method name: %s\n", 
            mulle_objc_universe_lookup_methodname(universe, method->descriptor.methodid));
        mulle_printf("Method signature: %s\n", method->descriptor.signature);
    }
    else
    {
        mulle_printf("ERROR: description method not found\n");
    }
    
    // Test class method lookup (should fail on infraclass)
    method = mulle_objc_infraclass_defaultsearch_method(infra,
        mulle_objc_methodid_from_string("computeValue"));
    
    if (method)
    {
        mulle_printf("ERROR: Found class method on infraclass\n");
    }
    else
    {
        mulle_printf("SUCCESS: computeValue not found on infraclass (expected)\n");
    }
    
    return 0;
}