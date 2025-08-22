// Test for Chapter 7: Ivar Access from Object
// Tests accessing instance variables directly from object instances

#import "include.h"
#include <stdio.h>
#include <string.h>

@interface TestClass
{
    int _count;
    char *_name;
}
- (int)count;
- (void)setCount:(int)value;
@end

@implementation TestClass
- (int)count { return _count; }
- (void)setCount:(int)value { _count = value; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *object;
    struct _mulle_objc_ivar *ivar;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    
    // Access _count ivar
    ivar = mulle_objc_infraclass_search_ivar(cls, 
                mulle_objc_ivarid_from_string("_count"));
    if (ivar) {
        *(int *)((char *)object + ivar->offset) = 42;
        printf("_count ivar offset: %d\n", ivar->offset);
        printf("_count value: %d\n", *(int *)((char *)object + ivar->offset));
    }
    
    // Access _name ivar
    ivar = mulle_objc_infraclass_search_ivar(cls, 
                mulle_objc_ivarid_from_string("_name"));
    if (ivar) {
        printf("_name ivar offset: %d\n", ivar->offset);
        printf("_name signature: %s\n", _mulle_objc_ivar_get_signature(ivar));
    }
    
    return 0;
}