// Test for Appendix A: Universe Structure Layout Inspection
// This test verifies the universe structure layout and field offsets

#import "include.h"
#include <stdio.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    printf("=== Universe Structure Layout Test ===\n");
    printf("Universe pointer: %p\n", (void *)universe);
    
    // Check field offsets by casting to char* and calculating differences
    printf("\nField offsets:\n");
    printf("version: %zu bytes\n", 
           (size_t)((char *)&universe->version - (char *)universe));
    printf("name: %zu bytes\n", 
           (size_t)((char *)&universe->name - (char *)universe));
    printf("classes: %zu bytes\n", 
           (size_t)((char *)&universe->classes - (char *)universe));
    printf("protocols: %zu bytes\n", 
           (size_t)((char *)&universe->protocols - (char *)universe));
    printf("allocator: %zu bytes\n", 
           (size_t)((char *)&universe->allocator - (char *)universe));
    
    printf("\nStructure sizes:\n");
    printf("sizeof(struct _mulle_objc_universe): %zu bytes\n", 
           sizeof(struct _mulle_objc_universe));
    printf("sizeof(struct _mulle_objc_class): %zu bytes\n", 
           sizeof(struct _mulle_objc_class));
    printf("sizeof(struct _mulle_objc_infraclass): %zu bytes\n", 
           sizeof(struct _mulle_objc_infraclass));
    printf("sizeof(struct _mulle_objc_method): %zu bytes\n", 
           sizeof(struct _mulle_objc_method));
    
    return 0;
}