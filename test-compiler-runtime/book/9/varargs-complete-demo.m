// Test for complete varargs object handling
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-vararg/mulle-vararg.h>

void process_objects(const char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    
    // Count objects
    int count = mulle_vararg_count_objects(args);
    printf("Found %d objects\n", count);
    
    // Reset and iterate
    va_end(args);
    va_start(args, format);
    
    struct _mulle_objc_object *obj;
    while ((obj = mulle_vararg_next_object(&args)))
    {
        printf("Object: %p\n", obj);
    }
    
    va_end(args);
}

int main(void)
{
    printf("Varargs object processing ready\n");
    return 0;
}