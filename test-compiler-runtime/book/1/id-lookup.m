// Test for Chapter 1: ID to Name Lookup
// Demonstrates converting runtime IDs back to names

#import "import.h"

@interface MyClass
@end
@implementation MyClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *my_class;
    mulle_objc_classid_t class_id;
    const char *name;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Convert name to ID and back
    class_id = mulle_objc_classid_from_string("MyClass");
    my_class = mulle_objc_universe_lookup_infraclass_nofail(universe, class_id);
    
    if (my_class) {
        name = _mulle_objc_infraclass_get_name(my_class);
        printf("0x%08x -> %s\n", class_id, name);
    }
    
    return 0;
}