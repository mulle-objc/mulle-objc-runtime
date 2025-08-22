// Test for Chapter 7: Basic Object Creation
// Tests creating objects from classes

#import "include.h"
#include <stdio.h>

@interface TestClass
{
    int _count;
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
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    printf("Object created: %p\n", object);
    printf("Object class: %s\n", 
           mulle_objc_class_get_name(_mulle_objc_object_get_isa(object)));
    
    return 0;
}