// Test for Chapter 7: Basic Object Size and Allocation
// Tests basic object allocation and size information

#import "include.h"
#include <stdio.h>

@interface TestClass
{
    int _value;
}
- (int)value;
- (void)setValue:(int)value;
@end

@implementation TestClass
- (int)value { return _value; }
- (void)setValue:(int)value { _value = value; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *object;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    printf("Object created: %p\n", object);
    printf("Object size: %zu bytes\n", 
           _mulle_objc_class_get_instancesize(&cls->base));
    
    return 0;
}