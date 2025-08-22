// Test for Chapter 9: Custom Universes
// Tests creating named runtime instances

#import "include.h"

int main(void)
{
    struct _mulle_objc_universe *default_universe;
    struct _mulle_objc_universe *custom_universe;
    
    default_universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("Test: Custom universe creation\n");
    
    mulle_printf("Default universe: %p\n", default_universe);
    
    // Create a custom universe
    custom_universe = mulle_objc_alloc_universe(mulle_objc_universeid_from_string("test"), "test");
    if (custom_universe)
    {
        mulle_printf("SUCCESS: Custom universe created\n");
        mulle_printf("Custom universe: %p\n", custom_universe);
        mulle_printf("Different from default: %s\n", 
                     custom_universe != default_universe ? "YES" : "NO");
        
        // Clean up
        mulle_objc_universe_destroy(custom_universe);
        mulle_printf("Custom universe destroyed\n");
    }
    else
    {
        mulle_printf("ERROR: Failed to create custom universe\n");
    }
    
    return 0;
}