// Test for Appendix A: Universe Structure Layout Inspection
// This test verifies the universe structure layout and field offsets

#import "include.h"
#include <stdio.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("=== Universe Structure Layout Test ===\n");
    mulle_printf("Universe pointer: %p\n", (void *)universe);
    
    // Check field offsets by casting to char* and calculating differences
    mulle_printf("\nField offsets:\n");
    mulle_printf("version: %zu bytes\n",
           (size_t)((char *)&universe->version - (char *)universe));
    mulle_printf("name: %zu bytes\n",
           (size_t)((char *)&universe->name - (char *)universe));
    mulle_printf("classes: %zu bytes\n",
           (size_t)((char *)&universe->classes - (char *)universe));
    mulle_printf("protocols: %zu bytes\n",
           (size_t)((char *)&universe->protocols - (char *)universe));
    mulle_printf("allocator: %zu bytes\n",
           (size_t)((char *)&universe->allocator - (char *)universe));
    
    mulle_printf("\nStructure sizes:\n");
    mulle_printf("sizeof(struct _mulle_objc_universe): %zu bytes\n",
           sizeof(struct _mulle_objc_universe));
    mulle_printf("sizeof(struct _mulle_objc_class): %zu bytes\n",
           sizeof(struct _mulle_objc_class));
    mulle_printf("sizeof(struct _mulle_objc_infraclass): %zu bytes\n",
           sizeof(struct _mulle_objc_infraclass));
    mulle_printf("sizeof(struct _mulle_objc_method): %zu bytes\n",
           sizeof(struct _mulle_objc_method));
    
    return 0;
}