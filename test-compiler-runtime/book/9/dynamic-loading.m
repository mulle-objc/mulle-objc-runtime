// Test for Chapter 9: Dynamic Loading
// Tests +load and +unload method execution

#import "include.h"

static int load_called = 0;
static int unload_called = 0;

@interface LoadTestClass
+ (void)load;
+ (void)unload;
+ (int)getLoadCount;
+ (int)getUnloadCount;
@end

@implementation LoadTestClass
+ (void)load
{
    load_called++;
    mulle_printf("LoadTestClass +load called\n");
}

+ (void)unload
{
    unload_called++;
    mulle_printf("LoadTestClass +unload called\n");
}

+ (int)getLoadCount { return load_called; }
+ (int)getUnloadCount { return unload_called; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("Test: Dynamic loading with +load/+unload\n");
    
    // Verify class is loaded and +load was called
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("LoadTestClass"));
    
    if (cls)
    {
        mulle_printf("SUCCESS: LoadTestClass found in runtime\n");
        mulle_printf("Load count: %d\n", [LoadTestClass getLoadCount]);
        mulle_printf("Unload count: %d\n", [LoadTestClass getUnloadCount]);
    }
    else
    {
        mulle_printf("ERROR: LoadTestClass not found\n");
    }
    
    return 0;
}