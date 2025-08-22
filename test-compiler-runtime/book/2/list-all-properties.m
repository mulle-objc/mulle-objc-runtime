// Test for Chapter 2: List All Properties
// Tests listing all declared properties of a class

#import "include.h"
#include <stdio.h>

@interface TestClass
@property int number;
@property (readonly) char *text;
@property double value;
@end
@implementation TestClass
@end

static mulle_objc_walkcommand_t print_property_name(struct _mulle_objc_property *property,
                                                    struct _mulle_objc_infraclass *infra,
                                                    void *userinfo)
{
    printf("  %s\n", _mulle_objc_property_get_name(property));
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    printf("Properties in TestClass:\n");
    _mulle_objc_infraclass_walk_properties(cls, 0, print_property_name, NULL);
    
    return 0;
}