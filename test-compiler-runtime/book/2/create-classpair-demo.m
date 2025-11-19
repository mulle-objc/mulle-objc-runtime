// Test for Chapter 2: Create Classpair Demo
// Based on actual working example from test/c/class/classpair.c

#import "import.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void)
{
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    struct _mulle_objc_classpair *pair;
    struct _mulle_objc_universe *universe;

    universe = mulle_objc_global_get_defaultuniverse();
    assert(universe);

    // Create both infraclass and metaclass together  
    pair = mulle_objc_universe_new_classpair(universe,
                                           mulle_objc_classid_from_string("DemoClass"),
                                           "DemoClass",
                                           0,    // instance size
                                           0,    // extra bytes  
                                           NULL); // no superclass
    assert(pair);

    // Extract the individual structs
    infra = _mulle_objc_classpair_get_infraclass(pair);
    meta  = _mulle_objc_classpair_get_metaclass(pair);

    // to be valid classes need at least an empty methodlist and an empty ivarlist, NULL doees that
    mulle_objc_class_add_methodlist( _mulle_objc_infraclass_as_class( infra), NULL);
    mulle_objc_class_add_methodlist( _mulle_objc_metaclass_as_class( meta), NULL);

    mulle_objc_infraclass_add_ivarlist( infra, NULL);
    mulle_objc_infraclass_add_propertylist( infra, NULL);

   //printf("Classpair creation demo:\n");
   //printf("Classpair: %p\n", (void *)pair);
   //printf("Infraclass: %p\n", (void *)infra);
   //printf("Metaclass: %p\n", (void *)meta);

    // Verify the bidirectional relationships
    if (pair != _mulle_objc_infraclass_get_classpair(infra))
        return 1;
    if (pair != _mulle_objc_metaclass_get_classpair(meta))
        return 1;
    if (meta != _mulle_objc_infraclass_get_metaclass(infra))
        return 1;
    if (infra != _mulle_objc_metaclass_get_infraclass(meta))
        return 1;

    printf("All relationships verified!\n");

    // Verify not present yet
    struct _mulle_objc_infraclass *missing = mulle_objc_universe_lookup_infraclass(
        universe, mulle_objc_classid_from_string("DemoClass"));
    if( missing != NULL)
    {
        fprintf( stderr, "DemoClass already exists!\n");
        return 1;
    }
    // Register with universe (metaclass auto-registered)
    mulle_objc_universe_register_infraclass_nofail(universe, infra);

    // Verify registration
    struct _mulle_objc_infraclass *found = mulle_objc_universe_lookup_infraclass(
        universe, mulle_objc_classid_from_string("DemoClass"));
    if (found != infra)
    {
        fprintf( stderr, "DemoClass is missing!\n");
        return 1;
    }

    printf("Registration successful!\n");

    // Since we registered, we must NOT free manually
    // mulle_objc_classpair_free(pair);
    
    return 0;
}