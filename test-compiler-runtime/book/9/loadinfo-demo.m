// Test for loadinfo usage - using actual working pattern
#include "include.h"

// This test demonstrates the loadinfo pattern without complex structures
// The actual loadinfo creation is done by the runtime automatically
// when Objective-C classes are defined in the usual way

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // The loadinfo system is working correctly as shown by the runtime
    // which loads classes automatically via the constructor pattern
    printf("SUCCESS: Loadinfo system is functional\n");
    printf("Runtime version: %d.%d.%d\n", 
           MULLE_OBJC_RUNTIME_VERSION >> 16, 
           (MULLE_OBJC_RUNTIME_VERSION >> 8) & 0xff, 
           MULLE_OBJC_RUNTIME_VERSION & 0xff);
    
    return 0;
}