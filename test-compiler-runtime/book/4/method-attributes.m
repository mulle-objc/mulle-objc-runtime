// Test for Chapter 4: Method Attributes
// Tests analyzing method type information and signatures

#import "include.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *test_class;
    struct _mulle_objc_method *method;
    
    universe = mulle_objc_global_get_defaultuniverse();
    test_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("NSObject")
    );
    
    mulle_printf("Testing method attributes:\n");
    
    // Test description method
    method = mulle_objc_class_search_method_nofail(
        (struct _mulle_objc_class *)test_class, 
        mulle_objc_uniqueid_from_string("description")
    );
    
    if (method)
    {
        const char *signature = _mulle_objc_method_get_signature(method);
        mulle_printf("  description: signature=%s\n", signature ? signature : "NULL");
    }
    else
    {
        mulle_printf("  description: method not found\n");
    }
    
    // Test init method
    method = mulle_objc_class_search_method_nofail(
        (struct _mulle_objc_class *)test_class, 
        mulle_objc_uniqueid_from_string("init")
    );
    
    if (method)
    {
        const char *signature = _mulle_objc_method_get_signature(method);
        mulle_printf("  init: signature=%s\n", signature ? signature : "NULL");
    }
    else
    {
        mulle_printf("  init: method not found\n");
    }
    
    mulle_printf("Method attributes test completed\n");
    return 0;
}