// Test for Chapter 5: Property Enumeration
// Tests walking through all properties of a class

#import "include.h"
#include <stdio.h>

@interface PropertyEnum
@property int id;
@property char *title;
@property double value;
@end

@implementation PropertyEnum
@end

static mulle_objc_walkcommand_t print_property(struct _mulle_objc_property *property,
                                               struct _mulle_objc_infraclass *infra,
                                               void *userinfo)
{
    printf("Property: %s (%s)\n", 
           _mulle_objc_property_get_name(property),
           _mulle_objc_property_get_signature(property));
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("PropertyEnum")
    );
    
    printf("All properties:\n");
    _mulle_objc_infraclass_walk_properties(cls, 0, print_property, NULL);
    
    return 0;
}