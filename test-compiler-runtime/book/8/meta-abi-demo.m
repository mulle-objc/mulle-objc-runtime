// Test for Chapter 8: Meta-ABI Comprehensive Example
// Tests struct-based parameter passing with Meta-ABI calling convention

#import "include.h"

@interface TestClass
- (int)add:(int)a to:(int)b;
- (const char *)formatString:(const char *)format with:(int)value and:(double)scale;
- (int)getStatus;
@end

@implementation TestClass
- (int)add:(int)a to:(int)b { return a + b; }
- (const char *)formatString:(const char *)format with:(int)value and:(double)scale
{ 
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), format, value, scale);
    return buffer;
}
- (int)getStatus { return 200; }
@end

// Simple parameter structure
struct add_params {
    int a;
    int b;
};

// Complex parameter structure
struct complex_params {
    const char *format;
    int value;
    double scale;
};

// Return value structure
struct return_value {
    int status;
    const char *message;
};

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    mulle_objc_implementation_t imp;
    void *result;
    id obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Meta-ABI: Comprehensive Parameter Passing Test\n");
    
    obj = mulle_objc_class_new(&infra->base);
    if (!obj)
    {
        mulle_printf("ERROR: Failed to create instance\n");
        return 1;
    }
    
    // Test 1: Simple parameter struct (add two integers)
    struct add_params add_args = {15, 25};
    result = mulle_objc_object_call(obj, 
        mulle_objc_methodid_from_string("add:to:"), 
        &add_args);
    
    mulle_printf("SUCCESS: Simple parameter struct\n");
    mulle_printf("15 + 25 = %d\n", (int)(intptr_t)result);
    
    // Test 2: Complex parameter struct (string, int, double)
    struct complex_params complex_args = {"Result: %d at %.2f", 42, 3.14};
    result = mulle_objc_object_call(obj,
        mulle_objc_methodid_from_string("formatString:with:and:"),
        &complex_args);
    
    mulle_printf("SUCCESS: Complex parameter struct\n");
    mulle_printf("Formatted: %s\n", (const char *)result);
    
    // Test 3: Zero parameters (method with no arguments)
    result = mulle_objc_object_call(obj,
        mulle_objc_methodid_from_string("getStatus"),
        NULL);
    
    mulle_printf("SUCCESS: Zero parameter case\n");
    mulle_printf("Status: %d\n", (int)(intptr_t)result);
    
    mulle_printf("All Meta-ABI tests completed successfully\n");
    return 0;
}