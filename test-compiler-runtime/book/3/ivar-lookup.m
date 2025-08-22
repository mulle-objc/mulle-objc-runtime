// Test for Chapter 3: Ivar Lookup
// Tests looking up instance variables by name

#import "include.h"
#include <stdio.h>

@interface Person {
    int    age;
    char   *name;
    double height;
}
@end

@implementation Person
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *person_class;
    struct _mulle_objc_ivar *ivar;
    
    universe = mulle_objc_global_get_defaultuniverse();
    person_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("Person")
    );
    
    // Test age ivar lookup
    ivar = _mulle_objc_infraclass_search_ivar(person_class, mulle_objc_uniqueid_from_string("age"));
    if (ivar)
    {
        printf("Found age ivar: %s\n", _mulle_objc_ivar_get_name(ivar));
        printf("Offset: %d\n", ivar->offset);
    }
    else
    {
        printf("age ivar not found\n");
        return 1;
    }
    
    // Test name ivar lookup
    ivar = _mulle_objc_infraclass_search_ivar(person_class, mulle_objc_uniqueid_from_string("name"));
    if (ivar)
    {
        printf("Found name ivar: %s\n", _mulle_objc_ivar_get_name(ivar));
        printf("Offset: %d\n", ivar->offset);
    }
    else
    {
        printf("name ivar not found\n");
        return 1;
    }
    
    // Test height ivar lookup
    ivar = _mulle_objc_infraclass_search_ivar(person_class, mulle_objc_uniqueid_from_string("height"));
    if (ivar)
    {
        printf("Found height ivar: %s\n", _mulle_objc_ivar_get_name(ivar));
        printf("Offset: %d\n", ivar->offset);
    }
    else
    {
        printf("height ivar not found\n");
        return 1;
    }
    
    // Test non-existent ivar
    ivar = _mulle_objc_infraclass_search_ivar(person_class, mulle_objc_uniqueid_from_string("nonexistent"));
    if (!ivar)
    {
        printf("Non-existent ivar correctly not found\n");
    }
    else
    {
        printf("Non-existent ivar unexpectedly found\n");
        return 1;
    }
    
    return 0;
}