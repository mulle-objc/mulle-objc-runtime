// Test for Chapter 5: Synthesized Properties
// Tests @synthesize directive for property implementation

#import "include.h"
#include <stdio.h>

@interface SynthProps
@property int value;
@property char *name;
@end

@implementation SynthProps
@synthesize value;
@synthesize name;
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("SynthProps")
    );
    
    printf("Synthesized properties class defined: %s\n", 
           _mulle_objc_class_get_name(&cls->base));
    
    return 0;
}