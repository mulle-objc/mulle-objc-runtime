// Test for Chapter 1: Minimal Runtime Access
// The most basic runtime interaction - getting the universe

#import "include.h"
#include <stdio.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    // Get the runtime context
    universe = mulle_objc_global_get_defaultuniverse();
    
    printf("Runtime ready: %p\n", (void *)universe);
    
    return 0;
}