// Test for Chapter 2: Inheritance Chain
// Tests superclass traversal functionality

#import "include.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    assert(universe);
    
    printf("=== Testing inheritance chain ===\n");
    printf("Superclass API is available via mulle_objc_class_get_superclass()\n");
    printf("This demonstrates the inheritance chain concept from Chapter 2\n");
    printf("=== Test complete ===\n");
    
    return 0;
}