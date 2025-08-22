// Test for Chapter 8: Variable Arguments Handling
// Tests methods with variable number of arguments

#import "include.h"
#include <stdarg.h>

@interface TestClass
- (int)sumValues:(int)count, ...;
- (const char *)formatValues:(const char *)format, ...;
@end

@implementation TestClass
- (int)sumValues:(int)count, ...
{
    va_list args;
    int sum = 0;
    
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    
    return sum;
}

- (const char *)formatValues:(const char *)format, ...
{
    va_list args;
    static char buffer[512];
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    return buffer;
}
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    id obj;
    int result;
    const char *str_result;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("TestClass"));
    
    mulle_printf("Test: Variable arguments handling\n");
    
    obj = mulle_objc_class_new(&infra->base);
    if (!obj)
    {
        mulle_printf("ERROR: Failed to create instance\n");
        return 1;
    }
    
    // Test varargs method with integers
    result = [obj sumValues:5, 1, 2, 3, 4, 5];
    mulle_printf("SUCCESS: Variable args sum\n");
    mulle_printf("Sum of 1+2+3+4+5 = %d\n", result);
    
    // Test varargs method with string formatting
    str_result = [obj formatValues:"Values: %d, %s, %f", 42, "test", 3.14];
    mulle_printf("Format result: %s\n", str_result);
    
    return 0;
}