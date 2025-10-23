// Test for object counting in varargs
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-vararg/mulle-vararg.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_object *obj1, *obj2, *obj3;
    int count;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Create test objects
    obj1 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("NSObject"));
    obj2 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("NSObject"));
    obj3 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("NSObject"));
    
    if (obj1 && obj2 && obj3)
    {
        // Count objects in varargs
        count = mulle_vararg_count_objects(obj1, obj2, obj3, NULL);
        printf("Counted %d objects in varargs\n", count);
    }
    
    return 0;
}