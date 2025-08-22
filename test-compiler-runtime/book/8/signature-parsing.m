// Test for Chapter 8: Signature System
// Tests method signature parsing and type encoding

#import "include.h"

@interface TestClass
- (int)simpleMethod;
- (const char *)stringMethod:(const char *)str;
- (id)complexMethod:(int)a withFloat:(float)b withString:(const char *)c;
+ (void)classMethod:(int)x;
@end

@implementation TestClass
- (int)simpleMethod { return 123; }
- (const char *)stringMethod:(const char *)str { return str; }
- (id)complexMethod:(int)a withFloat:(float)b withString:(const char *)c { return self; }
+ (void)classMethod:(int)x { }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_method *method;
    struct mulle_objc_signatureenumerator enumerator;
    struct mulle_objc_typeinfo info;
    char *signature;
    int count;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: Signature parsing and type encoding\n");
    
    // Test signature parsing for simple method
    method = mulle_objc_infraclass_defaultsearch_method(infra,
        mulle_objc_methodid_from_string("simpleMethod"));
    
    if (method)
    {
        signature = method->descriptor.signature;
        mulle_printf("simpleMethod signature: %s\n", signature);
        
        // Parse signature types
        enumerator = mulle_objc_signature_enumerate(signature);
        count = 0;
        while (_mulle_objc_signatureenumerator_next(&enumerator, &info))
        {
            mulle_printf("  Parameter %d: type='%s', size=%zu\n", 
                count++, info.type, info.natural_size);
        }
        _mulle_objc_signatureenumerator_rval(&enumerator, &info);
        mulle_printf("  Return type: '%s', size=%zu\n", info.type, info.natural_size);
        mulle_objc_signatureenumerator_done(&enumerator);
    }
    
    // Test complex method signature
    method = mulle_objc_infraclass_defaultsearch_method(infra,
        mulle_objc_methodid_from_string("complexMethod:withFloat:withString:"));
    
    if (method)
    {
        signature = method->descriptor.signature;
        mulle_printf("\ncomplexMethod signature: %s\n", signature);
        
        enumerator = mulle_objc_signature_enumerate(signature);
        count = 0;
        while (_mulle_objc_signatureenumerator_next(&enumerator, &info))
        {
            mulle_printf("  Parameter %d: type='%s', size=%zu\n", 
                count++, info.type, info.natural_size);
        }
        _mulle_objc_signatureenumerator_rval(&enumerator, &info);
        mulle_printf("  Return type: '%s', size=%zu\n", info.type, info.natural_size);
        mulle_objc_signatureenumerator_done(&enumerator);
    }
    
    return 0;
}