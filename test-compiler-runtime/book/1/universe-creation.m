// Test for universe creation API
// Tests mulle_objc_alloc_universe function

#import "include.h"

int main(void)
{
    struct _mulle_objc_universe *default_universe;
    struct _mulle_objc_universe *custom_universe;
    mulle_objc_universeid_t universe_id;
    
    mulle_printf("Testing universe creation API\n");
    
    // Get default universe
    default_universe = mulle_objc_global_get_defaultuniverse();
    mulle_printf("Default universe: %p\n", default_universe);
    
    // Create a custom universe using actual API
    universe_id = mulle_objc_universeid_from_string("test-universe");
    custom_universe = mulle_objc_alloc_universe(universe_id, "test-universe");
    
    if (custom_universe)
    {
        mulle_printf("SUCCESS: Custom universe created\n");
        mulle_printf("Custom universe: %p\n", custom_universe);
        mulle_printf("Different from default: %s\n", 
                     custom_universe != default_universe ? "YES" : "NO");
        
        // Verify universe ID and name
        mulle_printf("Custom universe ID: 0x%08x\n", custom_universe->universeid);
        mulle_printf("Custom universe name: %s\n", custom_universe->universename);
    }
    else
    {
        mulle_printf("ERROR: Failed to create custom universe\n");
        return 1;
    }
    
    return 0;
}