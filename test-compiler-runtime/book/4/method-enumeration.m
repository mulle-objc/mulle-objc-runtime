// Test for Chapter 4: Method Enumeration
// Tests enumerating all methods in a class hierarchy

#import "include.h"

@interface BaseClass
- (void)baseMethod;
- (int)baseValue;
@end

@implementation BaseClass
- (void)baseMethod { }
- (int)baseValue { return 42; }
@end

@interface DerivedClass : BaseClass
- (void)derivedMethod;
- (float)derivedValue;
@end

@implementation DerivedClass
- (void)derivedMethod { }
- (float)derivedValue { return 3.14f; }
@end

typedef struct {
    int count;
} method_context_t;

static mulle_objc_walkcommand_t print_method_name(struct _mulle_objc_method *method,
                                                  struct _mulle_objc_methodlist *list,
                                                  struct _mulle_objc_class *cls,
                                                  void *userinfo)
{
    method_context_t *context = (method_context_t *)userinfo;
    
    mulle_printf("  %s%s\n", 
           _mulle_objc_method_get_name(method),
           _mulle_objc_method_get_signature(method));
    context->count++;
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *base_class;
    struct _mulle_objc_infraclass *derived_class;
    method_context_t context;
    
    universe = mulle_objc_global_get_defaultuniverse();
    base_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("BaseClass")
    );
    derived_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("DerivedClass")
    );
    
    // Test base class method enumeration
    mulle_printf("BaseClass instance methods:\n");
    context.count = 0;
    _mulle_objc_class_walk_methods((struct _mulle_objc_class *)base_class, 0, print_method_name, &context);
    
    if (context.count != 2)
    {
        mulle_printf("Expected 2 instance methods in BaseClass, found %d\n", context.count);
        return 1;
    }
    
    // Test derived class method enumeration
    mulle_printf("\nDerivedClass instance methods:\n");
    context.count = 0;
    _mulle_objc_class_walk_methods((struct _mulle_objc_class *)derived_class, 0, print_method_name, &context);
    
    if (context.count != 4)  // 2 from BaseClass + 2 from DerivedClass
    {
        mulle_printf("Expected 4 instance methods in DerivedClass, found %d\n", context.count);
        return 1;
    }
    
    // Test class method enumeration
    mulle_printf("\nBaseClass class methods:\n");
    context.count = 0;
    struct _mulle_objc_metaclass *base_metaclass = _mulle_objc_class_get_metaclass((struct _mulle_objc_class *)base_class);
    _mulle_objc_class_walk_methods((struct _mulle_objc_class *)base_metaclass, 0, print_method_name, &context);
    
    mulle_printf("  Found %d class methods\n", context.count);
    
    mulle_printf("\nMethod enumeration test completed successfully\n");
    return 0;
}