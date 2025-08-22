// Test for Chapter 9: Category Loading and Method Injection
// Tests adding methods to existing classes via categories

#import "include.h"

@interface BaseClass
- (int)baseValue;
@end

@implementation BaseClass
- (int)baseValue { return 10; }
@end

// Category adding new methods
@interface BaseClass (Extension)
- (int)extendedValue;
+ (const char *)categoryName;
@end

@implementation BaseClass (Extension)
- (int)extendedValue { return 20; }
+ (const char *)categoryName { return "Extension"; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    id obj;
    int base_result, extended_result;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("BaseClass"));
    
    mulle_printf("Test: Category method injection\n");
    
    obj = mulle_objc_class_new(&infra->base);
    if (obj)
    {
        base_result = [obj baseValue];
        extended_result = [obj extendedValue];
        
        mulle_printf("SUCCESS: Category methods available\n");
        mulle_printf("Base method result: %d\n", base_result);
        mulle_printf("Category method result: %d\n", extended_result);
        mulle_printf("Category name: %s\n", [BaseClass categoryName]);
    }
    else
    {
        mulle_printf("ERROR: Failed to create instance\n");
    }
    
    return 0;
}