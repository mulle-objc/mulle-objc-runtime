//
// Test: Universe structure inspection
// Purpose: Verify universe structure layout and field access
//

#import <mulle-objc-runtime.h>
#include <assert.h>
#include <stdio.h>

int main()
{
    struct _mulle_objc_universe  *universe;
    
    universe = mulle_objc_get_defaultuniverse();
    assert( universe);
    
    // Test basic universe field access
    printf( "universe->version: %u\n", _mulle_objc_universe_get_version( universe));
    printf( "universe->path: %s\n", _mulle_objc_universe_get_path( universe));
    printf( "universe->universeid: %llu\n", (unsigned long long) _mulle_objc_universe_get_universeid( universe));
    printf( "universe->universename: %s\n", mulle_objc_universe_get_name( universe));
    
    // Test cache fill rate
    printf( "cache_fillrate: %u\n", _mulle_objc_universe_get_cache_fillrate( universe));
    
    // Verify universe is initialized
    assert( _mulle_objc_universe_is_initialized( universe));
    
    return 0;
}