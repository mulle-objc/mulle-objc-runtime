// Test for Chapter 2: Inspect Classpair
// Tests the classpair relationship between infraclass and metaclass

#import "include.h"
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

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
    
    meta = mulle_objc_class_get_metaclass(&infra->base);
    
    mulle_printf("Classpair inspection for TestClass:\n");
    mulle_printf("Infraclass: %p\n", (void *)infra);
    mulle_printf("Metaclass: %p\n", (void *)meta);
    mulle_printf("Same class name: %s\n",
           mulle_objc_class_get_name(&meta->base));
    mulle_printf("Infraclass has superclass: %s\n",
           mulle_objc_class_get_superclass(&infra->base) ? "yes" : "no");
    mulle_printf("Metaclass has superclass: %s\n",
           mulle_objc_class_get_superclass(&meta->base) ? "yes" : "no");
    
    return 0;
}