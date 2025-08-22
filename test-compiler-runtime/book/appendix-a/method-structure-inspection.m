//
// Test: Method structure inspection
// Purpose: Verify method structure layout and descriptor access
//

#import <mulle-objc-runtime.h>
#include <assert.h>
#include <stdio.h>

int main()
{
    struct _mulle_objc_universe  *universe;
    struct _mulle_objc_class     *cls;
    struct _mulle_objc_method    *method;
    
    universe = mulle_objc_get_defaultuniverse();
    assert( universe);
    
    // Get NSObject class
    cls = mulle_objc_universe_lookup_classid( universe, MULLE_OBJC_CLASSID_OBJECT);
    assert( cls);
    
    // Look up a method (init)
    method = mulle_objc_class_lookup_method( cls, MULLE_OBJC_METHODID_INIT);
    if( method)
    {
        printf( "method name: %s\n", mulle_objc_method_get_name( method));
        printf( "method signature: %s\n", mulle_objc_method_get_signature( method));
        printf( "method id: %llu\n", (unsigned long long) mulle_objc_method_get_methodid( method));
        
        // Test descriptor access
        struct _mulle_objc_descriptor *desc = mulle_objc_method_get_descriptor( method);
        printf( "descriptor methodid: %llu\n", (unsigned long long) desc->methodid);
        printf( "descriptor name: %s\n", desc->name);
        printf( "descriptor signature: %s\n", desc->signature);
        printf( "descriptor bits: 0x%08x\n", desc->bits);
        
        // Check method family
        enum _mulle_objc_methodfamily family = _mulle_objc_descriptor_get_methodfamily( desc);
        printf( "method family: %s\n", mulle_objc_methodfamily_utf8( family));
    }
    
    return 0;
}
