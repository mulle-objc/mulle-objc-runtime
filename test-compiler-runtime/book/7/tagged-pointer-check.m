// Test for Chapter 7: Tagged Pointer Check
// Tests detecting tagged pointers vs regular objects

#import "include.h"
#include <stdio.h>

@interface TestClass
{
    int _value;
}
- (int)value;
@end

@implementation TestClass
- (int)value { return _value; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *normal_object;
    void *tagged_pointer;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    normal_object = mulle_objc_infraclass_alloc_instance(cls);
    
    // Create a tagged pointer (simulated)
    tagged_pointer = (void *)0x7;
    
    printf("Normal object: %p\n", normal_object);
    printf("Normal object tagged index: %d\n", 
           mulle_objc_object_get_taggedpointerindex(normal_object));
    
    printf("Tagged pointer: %p\n", tagged_pointer);
    printf("Tagged pointer tagged index: %d\n", 
           mulle_objc_object_get_taggedpointerindex(tagged_pointer));
    
    return 0;
}