// Test for Chapter 10: Custom Universe Configuration
// Demonstrates registering a named universe with custom configuration

#include "include.h"
#include <stdio.h>

// User data for configuration
struct universe_config {
    const char *name;
    int cache_size;
};

// Custom bang callback
void custom_bang(struct _mulle_objc_universe *universe,
                 struct mulle_allocator *allocator,
                 void *userinfo)
{
    struct universe_config *config = userinfo;
    
    printf("Configuring universe '%s' with custom settings\n", 
           config ? config->name : "unknown");
    
    // Apply default configuration first
    _mulle_objc_universe_defaultbang(universe, allocator, userinfo);
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct universe_config config = { "MyApp", 2048 };
    mulle_objc_universeid_t universeid;
    
    // Create named universe with custom configuration
    universeid = mulle_objc_universeid_from_string("MyApp");
    universe = mulle_objc_global_register_universe(universeid, "MyApp");
    
    if (universe) {
        _mulle_objc_universe_bang(universe, custom_bang, NULL, &config);
        printf("Custom universe registered: %p\n", universe);
    }
    
    return 0;
}