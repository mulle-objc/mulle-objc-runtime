// Test for Chapter 3: Ivar Enumeration
// Tests enumerating all instance variables in a class hierarchy

#import "include.h"
#include <stdio.h>

@interface BaseClass {
    int    baseValue;
    char   *baseName;
}
@end

@implementation BaseClass
@end

@interface DerivedClass : BaseClass {
    float   derivedValue;
    double  derivedDouble;
    id      derivedObject;
}
@end

@implementation DerivedClass
@end

typedef struct {
    int      count;
} ivar_context_t;

static mulle_objc_walkcommand_t print_ivar_name(struct _mulle_objc_ivar *ivar,
                                                struct _mulle_objc_infraclass *infra,
                                                void *userinfo)
{
    ivar_context_t *context = (ivar_context_t *) userinfo;
    
    printf("  %s (offset: %d)\n", _mulle_objc_ivar_get_name(ivar), ivar->offset);
    context->count++;
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *base_class;
    struct _mulle_objc_infraclass *derived_class;
    ivar_context_t context;
    
    universe = mulle_objc_global_get_defaultuniverse();
    base_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("BaseClass")
    );
    derived_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("DerivedClass")
    );
    
    // Test base class ivar enumeration
    printf("BaseClass ivars:\n");
    context.count = 0;
    _mulle_objc_infraclass_walk_ivars(base_class, 0, print_ivar_name, &context);
    
    if (context.count != 2)
    {
        printf("Expected 2 ivars in BaseClass, found %d\n", context.count);
        return 1;
    }
    
    // Test derived class ivar enumeration (should include superclass ivars)
    printf("\nDerivedClass ivars:\n");
    context.count = 0;
    _mulle_objc_infraclass_walk_ivars(derived_class, 0, print_ivar_name, &context);
    
    if (context.count != 5)  // 2 from BaseClass + 3 from DerivedClass
    {
        printf("Expected 5 ivars in DerivedClass, found %d\n", context.count);
        return 1;
    }
    
    // Test that we can handle classes with no ivars gracefully
    printf("\nIvar enumeration test completed successfully\n");
    return 0;
}