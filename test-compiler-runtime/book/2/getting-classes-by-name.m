// Test for Chapter 2: Getting Classes by Name
// This tests the first example from dox/chapter2-class-system.md

#import "include.h"

@interface MyClass
@end
@implementation MyClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *my_class;
    
    universe = mulle_objc_global_get_defaultuniverse();
    my_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("MyClass")
    );
    
    mulle_printf("Test: Looking up MyClass by name\n");
    
    if (my_class)
    {
        mulle_printf("SUCCESS: Found MyClass class\n");
        mulle_printf("Class name: %s\n", _mulle_objc_infraclass_get_name(my_class));
    }
    else
    {
        mulle_printf("ERROR: MyClass not found\n");
    }
    
    return( 0);
}