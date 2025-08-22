// Test for Chapter 8: IMP Extraction and Invocation
// Tests extracting actual function pointers and calling them

#import "include.h"

@interface TestClass
- (int)compute:(int)value;
+ (const char *)staticCompute:(int)x;
@end

@implementation TestClass
- (int)compute:(int)value { return value * 2; }
+ (const char *)staticCompute:(int)x 
{ 
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "result: %d", x * 3);
    return buffer;
}
@end

// Define IMP signatures
typedef int (*IMP_instance_int_id)(id self, SEL _cmd, int param);
typedef const char *(*IMP_class_charp_id)(id self, SEL _cmd, int param);

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    struct _mulle_objc_method *method;
    id obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: IMP extraction and invocation\n");
    
    obj = mulle_objc_class_new(&infra->base);
    if (!obj)
    {
        mulle_printf("ERROR: Failed to create instance\n");
        return 1;
    }
    
    // Test instance method IMP extraction and calling
    method = mulle_objc_infraclass_defaultsearch_method(infra,
        mulle_objc_methodid_from_string("compute:"));
    
    if (method)
    {
        IMP_instance_int_id imp = (IMP_instance_int_id)_mulle_objc_method_get_implementation(method);
        
        mulle_printf("SUCCESS: Found compute: method\n");
        mulle_printf("IMP address: %p\n", imp);
        
        // Actually call the IMP
        int result = imp(obj, mulle_objc_methodid_from_string("compute:"), 21);
        mulle_printf("IMP call result: %d\n", result);
    }
    
    // Test class method IMP extraction and calling
    meta = mulle_objc_class_get_metaclass(&infra->base);
    method = mulle_objc_infraclass_defaultsearch_method(
        (struct _mulle_objc_infraclass *)meta,
        mulle_objc_methodid_from_string("staticCompute:"));
    
    if (method)
    {
        IMP_class_charp_id imp = (IMP_class_charp_id)_mulle_objc_method_get_implementation(method);
        
        mulle_printf("SUCCESS: Found staticCompute: method\n");
        mulle_printf("IMP address: %p\n", imp);
        
        // Actually call the IMP
        const char *result = imp((id)meta, 
            mulle_objc_methodid_from_string("staticCompute:"), 15);
        mulle_printf("IMP call result: %s\n", result);
    }
    
    return 0;
}