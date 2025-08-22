// Test for Chapter 1: Name to ID Conversion
// Demonstrates converting string names to runtime IDs

#import "include.h"
#include <stdio.h>

int main(void)
{
    mulle_objc_classid_t class_id;
    mulle_objc_methodid_t method_id;
    
    // Convert class name to ID
    class_id = mulle_objc_classid_from_string("NSObject");
    printf("NSObject -> 0x%08x\n", class_id);
    
    // Convert method name to ID
    method_id = mulle_objc_methodid_from_string("description");
    printf("description -> 0x%08x\n", method_id);
    
    return 0;
}