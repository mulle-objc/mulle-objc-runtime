// Test for Chapter 5: Property Lookup
// Tests runtime property lookup by name

#import "include.h"
#include <stdio.h>

@interface PropertyLookup
@property int count;
@property char *name;
@end

@implementation PropertyLookup
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_property *prop;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("PropertyLookup")
    );
    
    prop = _mulle_objc_infraclass_search_property(cls, 
        mulle_objc_propertyid_from_string("count"));
    if (prop) {
        printf("Found property: %s\n", _mulle_objc_property_get_name(prop));
        printf("Type: %s\n", _mulle_objc_property_get_signature(prop));
    }
    
    return 0;
}