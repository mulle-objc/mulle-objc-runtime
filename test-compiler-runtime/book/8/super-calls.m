// Test for Chapter 8: Super Calls
// Tests bypassing current class for method lookup

#import "include.h"

@interface BaseClass
- (int)getValue;
@end

@implementation BaseClass
- (int)getValue { return 100; }
@end

@interface DerivedClass : BaseClass
- (int)getValue;
- (int)getSuperValue;
@end

@implementation DerivedClass
- (int)getValue { return 200; }
- (int)getSuperValue { return [super getValue]; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *derived;
    id obj;
    int direct_value, super_value;
    
    universe = mulle_objc_global_get_defaultuniverse();
    derived = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("DerivedClass"));
    
    mulle_printf("Test: Super method calls\n");
    
    obj = mulle_objc_class_new(&derived->base);
    if (obj)
    {
        direct_value = [obj getValue];
        super_value = [obj getSuperValue];
        
        mulle_printf("SUCCESS: Super method dispatch worked\n");
        mulle_printf("Direct value: %d\n", direct_value);
        mulle_printf("Super value: %d\n", super_value);
    }
    else
    {
        mulle_printf("ERROR: Failed to create instance\n");
    }
    
    return 0;
}