// Test for Chapter 2: Inspect Metaclass Relationship
// Tests the relationship between infraclass and metaclass

#import "include.h"
#include <stdio.h>

@interface TestClass
- (int)instanceMethod;
+ (int)classMethod;
@end
@implementation TestClass
- (int)instanceMethod { return 42; }
+ (int)classMethod { return 84; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    // Get metaclass from infraclass
    meta = mulle_objc_class_get_metaclass(&infra->base);
    
    printf("Classpair analysis for TestClass:\n");
    printf("Infraclass address: %p\n", (void *)infra);
    printf("Metaclass address: %p\n", (void *)meta);
    printf("Both share name: %s\n", mulle_objc_class_get_name(&meta->base));
    
    // Show inheritance chains
    struct _mulle_objc_class *super_infra = mulle_objc_class_get_superclass(&infra->base);
    struct _mulle_objc_class *super_meta = mulle_objc_class_get_superclass(&meta->base);
    
    printf("Infraclass superclass: %s\n", 
           super_infra ? mulle_objc_class_get_name(super_infra) : "none");
    printf("Metaclass superclass: %s\n", 
           super_meta ? mulle_objc_class_get_name(super_meta) : "none");
    
    // Demonstrate the classpair concept
    struct _mulle_objc_classpair *pair = _mulle_objc_infraclass_get_classpair(infra);
    printf("Classpair structure: %p\n", (void *)pair);
    printf("Infraclass from pair: %p\n", (void *)_mulle_objc_classpair_get_infraclass(pair));
    printf("Metaclass from pair: %p\n", (void *)_mulle_objc_classpair_get_metaclass(pair));
    
    return 0;
}