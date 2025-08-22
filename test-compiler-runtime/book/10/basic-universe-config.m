// Test for Chapter 10: Universe Configuration
// Demonstrates basic universe configuration using _mulle_objc_universe_bang

#include "include.h"
#include <stdio.h>

// Custom bang callback for configuration
void my_universe_bang(struct _mulle_objc_universe *universe,
                     struct mulle_allocator *allocator,
                     void *userinfo)
{
    printf("Configuring universe %p\n", universe);
    
    // Use default configuration
    _mulle_objc_universe_defaultbang(universe, allocator, userinfo);
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    // Get default universe and apply custom configuration
    universe = mulle_objc_global_get_defaultuniverse();
    
    if (universe) {
        _mulle_objc_universe_bang(universe, my_universe_bang, NULL, NULL);
        printf("Universe configured: %p\n", universe);
    }
    
    return 0;
}