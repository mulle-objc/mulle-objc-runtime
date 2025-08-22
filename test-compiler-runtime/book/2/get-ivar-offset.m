// Test for Chapter 2: Get Ivar Offset
// Tests getting ivar offset by name

#import "include.h"
#include <stdio.h>

@interface TestClass {
    int number;
    char *text;
}
@end
@implementation TestClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_ivar *ivar;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    ivar = _mulle_objc_infraclass_search_ivar(cls, mulle_objc_uniqueid_from_string("number"));
    if (ivar) {
        printf("Offset of 'number': %d bytes\n", ivar->offset);
    }
    
    ivar = _mulle_objc_infraclass_search_ivar(cls, mulle_objc_uniqueid_from_string("text"));
    if (ivar) {
        printf("Offset of 'text': %d bytes\n", ivar->offset);
    }
    
    return 0;
}