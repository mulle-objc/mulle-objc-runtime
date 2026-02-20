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
    
    mulle_printf("Classpair analysis for TestClass:\n");
    mulle_printf("Infraclass address: %p\n", (void *)infra);
    mulle_printf("Metaclass address: %p\n", (void *)meta);
    mulle_printf("Both share name: %s\n", mulle_objc_class_get_name(&meta->base));
    
    // Show inheritance chains
    struct _mulle_objc_class *super_infra = mulle_objc_class_get_superclass(&infra->base);
    struct _mulle_objc_class *super_meta = mulle_objc_class_get_superclass(&meta->base);
    
    mulle_printf("Infraclass superclass: %s\n",
           super_infra ? mulle_objc_class_get_name(super_infra) : "none");
    mulle_printf("Metaclass superclass: %s\n",
           super_meta ? mulle_objc_class_get_name(super_meta) : "none");
    
    // Demonstrate the classpair concept
    struct _mulle_objc_classpair *pair = _mulle_objc_infraclass_get_classpair(infra);
    mulle_printf("Classpair structure: %p\n", (void *)pair);
    mulle_printf("Infraclass from pair: %p\n", (void *)_mulle_objc_classpair_get_infraclass(pair));
    mulle_printf("Metaclass from pair: %p\n", (void *)_mulle_objc_classpair_get_metaclass(pair));
    
    return 0;
}