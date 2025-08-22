// Test for Chapter 7: Instance Size Inspection
// Tests getting instance size information from objects

#import "include.h"
#include <stdio.h>

@interface SmallClass
{
    int _value;
}
@end

@implementation SmallClass
@end

@interface LargeClass
{
    int _int1;
    int _int2;
    int _int3;
    char *_string;
    void *_ptr;
}
@end

@implementation LargeClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *small_cls, *large_cls;
    void *small_obj, *large_obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Create small object
    small_cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("SmallClass")
    );
    small_obj = mulle_objc_infraclass_alloc_instance(small_cls);
    
    // Create large object
    large_cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("LargeClass")
    );
    large_obj = mulle_objc_infraclass_alloc_instance(large_cls);
    
    printf("SmallClass instance size: %zu bytes\n", 
           mulle_objc_class_get_instancesize(&small_cls->base));
    printf("LargeClass instance size: %zu bytes\n", 
           mulle_objc_class_get_instancesize(&large_cls->base));
    
    printf("Small object: %p\n", small_obj);
    printf("Large object: %p\n", large_obj);
    
    return 0;
}