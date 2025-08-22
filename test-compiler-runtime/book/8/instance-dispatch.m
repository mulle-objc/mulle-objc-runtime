// Test for Chapter 8: Instance Method Dispatch
// Tests regular method dispatch through instance

#import "include.h"

@interface TestClass
- (int)add:(int)a to:(int)b;
@end

@implementation TestClass
- (int)add:(int)a to:(int)b { return a + b; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    id obj;
    int result;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: Instance method dispatch\n");
    
    obj = mulle_objc_class_new(&infra->base);
    if (obj)
    {
        result = (int) [obj add:5 to:7];
        mulle_printf("SUCCESS: Method dispatch worked\n");
        mulle_printf("Result: 5 + 7 = %d\n", result);
    }
    else
    {
        mulle_printf("ERROR: Failed to create instance\n");
    }
    
    return 0;
}