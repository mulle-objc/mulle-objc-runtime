// Test for Chapter 4: Method Inheritance
// Tests method lookup through inheritance hierarchy

#import "include.h"
#include <stdio.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *ns_object;
    struct _mulle_objc_infraclass *ns_string;
    struct _mulle_objc_method *method;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Get classes through inheritance hierarchy
    ns_object = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("NSObject")
    );
    
    ns_string = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("NSString")
    );
    
    mulle_printf("Testing method inheritance:\n");
    
    // Test NSObject methods
    mulle_printf("NSObject methods:\n");
    method = mulle_objc_class_search_method_nofail(
        (struct _mulle_objc_class *)ns_object, 
        mulle_objc_uniqueid_from_string("description")
    );
    mulle_printf("  description: %s\n", method ? "found" : "not found");
    
    method = mulle_objc_class_search_method_nofail(
        (struct _mulle_objc_class *)ns_object, 
        mulle_objc_uniqueid_from_string("init")
    );
    mulle_printf("  init: %s\n", method ? "found" : "not found");
    
    // Test NSString methods
    mulle_printf("\nNSString methods:\n");
    method = mulle_objc_class_search_method_nofail(
        (struct _mulle_objc_class *)ns_string, 
        mulle_objc_uniqueid_from_string("description")
    );
    mulle_printf("  description (inherited): %s\n", method ? "found" : "not found");
    
    method = mulle_objc_class_search_method_nofail(
        (struct _mulle_objc_class *)ns_string, 
        mulle_objc_uniqueid_from_string("length")
    );
    mulle_printf("  length (NSString specific): %s\n", method ? "found" : "not found");
    
    mulle_printf("Method inheritance test completed\n");
    return 0;
}