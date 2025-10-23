// Test for object iteration in varargs
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-vararg/mulle-vararg.h>

void process_object_list(va_list args)
{
    struct _mulle_objc_object *obj;
    
    while ((obj = mulle_vararg_next_object(&args)))
    {
        printf("Processing object: %p\n", obj);
        // Process each object
    }
}

int main(void)
{
    printf("Object iteration ready\n");
    printf("Use mulle_vararg_next_object() to iterate objects\n");
    
    return 0;
}