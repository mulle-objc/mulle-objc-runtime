// Test for Chapter 8: Class Method Dispatch
// Tests metaclass method dispatch

#import "include.h"

@interface TestClass
+ (const char *)className;
+ (int)classValue;
@end

@implementation TestClass
+ (const char *)className { return "TestClass"; }
+ (int)classValue { return 99; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    const char *name;
    int value;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: Class method dispatch through metaclass\n");
    
    meta = mulle_objc_class_get_metaclass(&infra->base);
    
    name = [TestClass className];
    value = [TestClass classValue];
    
    mulle_printf("SUCCESS: Class method dispatch worked\n");
    mulle_printf("Class name: %s\n", name);
    mulle_printf("Class value: %d\n", value);
    
    return 0;
}