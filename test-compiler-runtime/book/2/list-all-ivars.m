// Test for Chapter 2: List All Ivars
// Tests listing all instance variables of a class

#import "include.h"
#include <stdio.h>

@interface TestClass {
    int number;
    char *text;
    double value;
}
@end
@implementation TestClass
@end

static mulle_objc_walkcommand_t print_ivar_name(struct _mulle_objc_ivar *ivar,
                                                struct _mulle_objc_infraclass *infra,
                                                void *userinfo)
{
    printf("  %s (offset: %d)\n", _mulle_objc_ivar_get_name(ivar), ivar->offset);
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
    
    printf("Instance variables in TestClass:\n");
    _mulle_objc_infraclass_walk_ivars(cls, 0, print_ivar_name, NULL);
    
    return 0;
}