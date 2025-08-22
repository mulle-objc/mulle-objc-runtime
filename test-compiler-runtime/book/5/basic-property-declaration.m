// Test for Chapter 5: Basic Property Declaration
// Tests basic property creation and access

#import "include.h"
#include <stdio.h>

@interface PropertyClass
@property int value;
@end

@implementation PropertyClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("PropertyClass")
    );
    
    printf("Property class defined: %s\n", _mulle_objc_class_get_name(&cls->base));
    
    return 0;
}