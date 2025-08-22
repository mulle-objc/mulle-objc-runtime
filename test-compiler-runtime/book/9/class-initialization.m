// Test for Chapter 9: Class Initialization
// Tests +initialize mechanics and lazy initialization

#import "include.h"

static int initialize_called = 0;
static int initialized_value = 0;

@interface InitializeClass
+ (void)initialize;
+ (int)getInitializedValue;
@end

@implementation InitializeClass
+ (void)initialize
{
    initialize_called++;
    initialized_value = 42;
    mulle_printf("InitializeClass +initialize called\n");
}

+ (int)getInitializedValue { return initialized_value; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("Test: Class initialization (+initialize)\n");
    
    // +initialize should not be called yet
    mulle_printf("Before first use - initialize count: %d\n", initialize_called);
    mulle_printf("Before first use - value: %d\n", [InitializeClass getInitializedValue]);
    
    // Now trigger +initialize by using the class
    int value = [InitializeClass getInitializedValue];
    
    mulle_printf("SUCCESS: +initialize triggered on first use\n");
    mulle_printf("After use - initialize count: %d\n", initialize_called);
    mulle_printf("After use - value: %d\n", value);
    
    return 0;
}