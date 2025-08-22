// Test for Chapter 3: Ivar Attributes
// Tests analyzing instance variable type information and signatures

#import "include.h"
#include <stdio.h>
#include <string.h>

@interface TestClass {
    int     intValue;
    float   floatValue;
    double  doubleValue;
    char    *stringValue;
    id      objectValue;
    struct  {
        int x;
        int y;
    } point;
}
@end

@implementation TestClass
@end

typedef struct {
    const char *name;
    const char *expected_encoding;
} ivar_test_t;

static ivar_test_t test_ivars[] = {
    { "intValue",    "i" },
    { "floatValue",  "f" },
    { "doubleValue", "d" },
    { "stringValue", "*" },
    { "objectValue", "@" },
    { "point",       "{" },
    { NULL, NULL }
};

static const char *get_type_name(const char *encoding)
{
    if (!encoding) return "unknown";
    
    switch (encoding[0])
    {
        case 'i': return "int";
        case 'f': return "float";
        case 'd': return "double";
        case '*': return "char *";
        case '@': return "id";
        case '{': return "struct";
        default:  return "unknown";
    }
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *test_class;
    struct _mulle_objc_ivar *ivar;
    ivar_test_t *test;
    
    universe = mulle_objc_global_get_defaultuniverse();
    test_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass")
    );
    
    printf("Testing ivar attributes:\n");
    printf("%-15s %-15s %-15s %-15s\n", 
           "Name", "Signature", "Type", "Offset");
    printf("%-15s %-15s %-15s %-15s\n", 
           "---------------", "---------------", "---------------", "---------------");
    
    for (test = test_ivars; test->name; test++)
    {
        ivar = _mulle_objc_infraclass_search_ivar(test_class, mulle_objc_uniqueid_from_string((char *)test->name));
        if (!ivar)
        {
            printf("ERROR: %s ivar not found\n", test->name);
            return 1;
        }
        
        const char *signature = _mulle_objc_ivar_get_signature(ivar);
        int offset = _mulle_objc_ivar_get_offset(ivar);
        
        printf("%-15s %-15s %-15s %-15d\n", 
               test->name, 
               signature ? signature : "NULL", 
               get_type_name(signature),
               offset);
        
        // Validate encoding starts with expected character
        if (signature && signature[0] != test->expected_encoding[0])
        {
            printf("ERROR: Expected encoding starting with '%c', got '%s'\n", 
                   test->expected_encoding[0], signature);
            return 1;
        }
    }
    
    // Test signature parsing for complex types
    printf("\nComplex signature analysis:\n");
    ivar = _mulle_objc_infraclass_search_ivar(test_class, mulle_objc_uniqueid_from_string("point"));
    if (ivar)
    {
        const char *signature = _mulle_objc_ivar_get_signature(ivar);
        printf("Point signature: %s\n", signature ? signature : "NULL");
        
        // Find struct opening
        if (signature)
        {
            const char *struct_start = strchr(signature, '{');
            if (struct_start)
            {
                const char *struct_end = strchr(struct_start, '}');
                if (struct_end)
                {
                    printf("Struct definition: %.*s\n", 
                           (int)(struct_end - struct_start + 1), struct_start);
                }
            }
        }
    }
    
    // Test offset calculation for direct access
    printf("\nTesting offset calculation:\n");
    for (test = test_ivars; test->name; test++)
    {
        ivar = _mulle_objc_infraclass_search_ivar(test_class, mulle_objc_uniqueid_from_string((char *)test->name));
        if (ivar)
        {
            printf("ivar '%s' is at offset %d bytes from object start\n", 
                   _mulle_objc_ivar_get_name(ivar), ivar->offset);
        }
    }
    
    printf("\nIvar attributes test completed successfully\n");
    return 0;
}