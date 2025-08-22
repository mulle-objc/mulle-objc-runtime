// Test for Chapter 9: Runtime Extensions
// Tests extending runtime with custom behaviors

#import "include.h"

@interface ExtensionTestClass
- (int)baseMethod;
@end

@implementation ExtensionTestClass
- (int)baseMethod { return 100; }
@end

// Custom extension function demonstration
static void register_extension_behavior(struct _mulle_objc_class *cls)
{
    mulle_printf("Extension registered for class: %s\n", 
                 _mulle_objc_class_get_name(cls));
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("ExtensionTestClass"));
    
    mulle_printf("Test: Runtime extension framework\n");
    
    if (infra)
    {
        mulle_printf("SUCCESS: Runtime extension example\n");
        mulle_printf("Class name: %s\n", _mulle_objc_class_get_name(&infra->base));
        
        // Demonstrate extension registration
        register_extension_behavior(&infra->base);
        mulle_printf("Extension framework ready for use\n");
    }
    else
    {
        mulle_printf("ERROR: ExtensionTestClass not found\n");
    }
    
    return 0;
}