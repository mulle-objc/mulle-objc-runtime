// Test for Chapter 5: Property Attributes
// Tests different property attribute combinations

#import "include.h"
#include <stdio.h>

@interface PropertyAttrs
@property int assignProp;
@property char *name;
@property (readonly) int readOnly;
@end

@implementation PropertyAttrs
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("PropertyAttrs")
    );
    
    printf("Property attributes class defined: %s\n", 
           _mulle_objc_class_get_name(&cls->base));
    
    return 0;
}