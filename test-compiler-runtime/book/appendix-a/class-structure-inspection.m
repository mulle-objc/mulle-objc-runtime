//
// Test: Class structure inspection
// Purpose: Verify class structure layout and distinguish infraclass vs metaclass
//

#import <mulle-objc-runtime.h>
#include <assert.h>
#include <stdio.h>

int main()
{
    struct _mulle_objc_universe  *universe;
    struct _mulle_objc_class     *cls;
    struct _mulle_objc_class     *metaclass;
    
    universe = mulle_objc_get_defaultuniverse();
    assert( universe);
    
    // Get NSObject class
    cls = mulle_objc_universe_lookup_classid( universe, MULLE_OBJC_CLASSID_OBJECT);
    assert( cls);
    
    // Test basic class field access
    mulle_printf( "class name: %s\n", _mulle_objc_class_get_name( cls));
    mulle_printf( "class id: %llu\n", (unsigned long long) _mulle_objc_class_get_classid( cls));
    mulle_printf( "class allocationsize: %zu\n", _mulle_objc_class_get_allocationsize( cls));
    
    // Check if it's an infraclass (infraclass == NULL)
    if( _mulle_objc_class_is_infraclass( cls))
    {
        mulle_printf( "This is an infraclass\n");
    }
    
    // Get metaclass
    metaclass = mulle_objc_class_get_metaclass( cls);
    if( metaclass)
    {
        mulle_printf( "metaclass name: %s\n", _mulle_objc_class_get_name( metaclass));
        mulle_printf( "metaclass is metaclass: %d\n", _mulle_objc_class_is_metaclass( metaclass));
    }
    
    // Test superclass chain
    struct _mulle_objc_class *super = _mulle_objc_class_get_superclass( cls);
    if( super)
    {
        mulle_printf( "superclass name: %s\n", _mulle_objc_class_get_name( super));
    }
    
    return 0;
}
