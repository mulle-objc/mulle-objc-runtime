// Test for Chapter 1: Universe Access
// Demonstrates accessing default and thread-local universes

#import "include.h"
#include <stdio.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    // Get the default universe
    universe = mulle_objc_global_get_defaultuniverse();
    
    if (universe) {
        printf("Default universe: %p\n", (void *)universe);
    }
    
    return 0;
}