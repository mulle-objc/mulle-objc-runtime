// Test for Chapter 2: Inspect Property
// Tests examining individual property attributes

#import "include.h"
#include <stdio.h>

@interface TestClass
@property int number;
@property (readonly) char *text;
@property double value;
@end
@implementation TestClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_property *prop;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    prop = _mulle_objc_infraclass_search_property(cls, 
        mulle_objc_propertyid_from_string("text"));
    if (prop) {
        printf("Property: %s\n", _mulle_objc_property_get_name(prop));
        printf("Type: %s\n", _mulle_objc_property_get_signature(prop));
    }
    
    return 0;
}