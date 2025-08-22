// Test for Chapter 2: Inspect Class Size
// Tests the inspect_class_layout function

#import "include.h"
#include <stdio.h>

@interface MyClass
{
    int _count;
    char _name[32];
    void *_data;
}
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
    
    if (my_class)
    {
        printf("Class: %s\n", mulle_objc_class_get_name(&my_class->base));
        printf("Instance size: %zu bytes\n", mulle_objc_class_get_instancesize(&my_class->base));
        printf("Ivar size: %zu bytes\n", _mulle_objc_class_get_instancesize(&my_class->base));
        return 0;
    }
    else
    {
        printf("ERROR: MyClass not found\n");
        return 1;
    }
}