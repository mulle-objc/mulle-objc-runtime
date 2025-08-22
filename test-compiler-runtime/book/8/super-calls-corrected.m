// Test for Chapter 8: Super Calls (Corrected)
// Tests bypassing current class with proper mulle_objc_searcharguments_make_super API

#import "include.h"

@interface BaseClass
- (int)getValue;
- (const char *)getName;
@end

@implementation BaseClass
- (int)getValue { return 100; }
- (const char *)getName { return "BaseClass"; }
@end

@interface DerivedClass : BaseClass
- (int)getValue;
- (int)getSuperValue;
- (const char *)getSuperName;
@end

@implementation DerivedClass
- (int)getValue { return 200; }
- (int)getSuperValue 
{ 
    // This demonstrates the proper super call mechanism
    struct _mulle_objc_class *cls = mulle_objc_object_get_class(self);
    struct _mulle_objc_searcharguments args;
    struct _mulle_objc_method *method;
    mulle_objc_implementation_t imp;
    
    args = mulle_objc_searcharguments_make_super(
        mulle_objc_methodid_from_string("getValue"),
        mulle_objc_class_get_classid(cls));
    
    method = mulle_objc_class_search_method(cls, &args, cls->inheritance, NULL);
    if (method)
    {
        imp = _mulle_objc_method_get_implementation(method);
        return (int)(intptr_t) mulle_objc_implementation_invoke(imp, self, 
            mulle_objc_methodid_from_string("getValue"), self);
    }
    return 0;
}
- (const char *)getSuperName 
{ 
    struct _mulle_objc_class *cls = mulle_objc_object_get_class(self);
    struct _mulle_objc_searcharguments args;
    struct _mulle_objc_method *method;
    mulle_objc_implementation_t imp;
    
    args = mulle_objc_searcharguments_make_super(
        mulle_objc_methodid_from_string("getName"),
        mulle_objc_class_get_classid(cls));
    
    method = mulle_objc_class_search_method(cls, &args, cls->inheritance, NULL);
    if (method)
    {
        imp = _mulle_objc_method_get_implementation(method);
        return (const char *) mulle_objc_implementation_invoke(imp, self, 
            mulle_objc_methodid_from_string("getName"), self);
    }
    return NULL;
}
@end

// Helper function demonstrating super call pattern
static void test_super_call(id obj, const char *method_name)
{
    struct _mulle_objc_class *cls = mulle_objc_object_get_class(obj);
    struct _mulle_objc_searcharguments args;
    struct _mulle_objc_method *method;
    mulle_objc_implementation_t imp;
    
    args = mulle_objc_searcharguments_make_super(
        mulle_objc_methodid_from_string(method_name),
        mulle_objc_class_get_classid(cls));
    
    method = mulle_objc_class_search_method(cls, &args, cls->inheritance, NULL);
    if (method)
    {
        imp = _mulle_objc_method_get_implementation(method);
        mulle_objc_implementation_invoke(imp, obj, 
            mulle_objc_methodid_from_string(method_name), obj);
    }
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *derived;
    id obj;
    int direct_value, super_value;
    const char *super_name;
    
    universe = mulle_objc_global_get_defaultuniverse();
    derived = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("DerivedClass"));
    
    mulle_printf("Test: Super method calls with correct API\n");
    
    obj = mulle_objc_class_new(&derived->base);
    if (obj)
    {
        direct_value = [obj getValue];
        super_value = [obj getSuperValue];
        super_name = [obj getSuperName];
        
        mulle_printf("SUCCESS: Super method dispatch worked\n");
        mulle_printf("Direct value: %d\n", direct_value);
        mulle_printf("Super value: %d\n", super_value);
        mulle_printf("Super name: %s\n", super_name);
        
        // Test direct super call pattern
        test_super_call(obj, "getValue");
    }
    else
    {
        mulle_printf("ERROR: Failed to create instance\n");
    }
    
    return 0;
}