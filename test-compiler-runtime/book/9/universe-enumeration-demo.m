// Test for universe-wide class enumeration
#include <mulle-objc-runtime/mulle-objc-runtime.h>

static void print_class_name(struct _mulle_objc_infraclass *cls, void *userinfo)
{
    printf("Class: %s\n", mulle_objc_class_get_name(cls));
}

int main(void)
{
    struct _mulle_objc_universe   *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Enumerate all classes
    mulle_objc_universe_walk_infraclasses(universe, print_class_name, NULL);
    
    return 0;
}