// Test for Chapter 5: Dynamic Properties
// Tests property declaration without ivars

#import "include.h"
#include <stdio.h>

@interface DynamicProps
@property int computed;
@end

@implementation DynamicProps
@dynamic computed;
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("DynamicProps")
    );
    
    printf("Dynamic property class defined: %s\n", 
           _mulle_objc_class_get_name(&cls->base));
    
    return 0;
}