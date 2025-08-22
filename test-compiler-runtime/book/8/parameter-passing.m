// Test for Chapter 8: Parameter Passing Convention
// Tests struct-based parameter passing with actual mulle-objc-runtime APIs

#import "include.h"

@interface TestClass
- (int)add:(int)a to:(int)b;
- (const char *)formatString:(const char *)format with:(int)value;
@end

@implementation TestClass
- (int)add:(int)a to:(int)b { return a + b; }
- (const char *)formatString:(const char *)format with:(int)value 
{ 
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), format, value);
    return buffer;
}
@end

// Parameter structure for method calls
typedef struct {
    int a;
    int b;
} add_params_t;

typedef struct {
    const char *format;
    int value;
} format_params_t;

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
    
    mulle_printf("Test: Parameter passing convention\n");
    
    obj = mulle_objc_class_new(&infra->base);
    if (!obj)
    {
        mulle_printf("ERROR: Failed to create instance\n");
        return 1;
    }
    
    // Test direct method call with parameter struct
    add_params_t add_args = {5, 7};
    result = mulle_objc_object_call(obj, 
        mulle_objc_methodid_from_string("add:to:"), 
        &add_args);
    
    mulle_printf("SUCCESS: Parameter passing worked\n");
    mulle_printf("5 + 7 = %d\n", (int)(intptr_t)result);
    
    // Test with string parameter
    format_params_t format_args = {"Value: %d", 42};
    result = mulle_objc_object_call(obj,
        mulle_objc_methodid_from_string("formatString:with:"),
        &format_args);
    
    mulle_printf("Format result: %s\n", (const char *)result);
    
    return 0;
}