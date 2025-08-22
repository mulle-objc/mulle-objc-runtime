// Test for Chapter 7: Get Class from Object
// Tests getting class information from object instances

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
    struct _mulle_objc_class *object_class;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    object_class = _mulle_objc_object_get_isa(object);
    
    printf("Object: %p\n", object);
    printf("Class name: %s\n", mulle_objc_class_get_name(object_class));
    printf("Instance size: %zu bytes\n", 
           mulle_objc_class_get_instancesize(object_class));
    
    return 0;
}