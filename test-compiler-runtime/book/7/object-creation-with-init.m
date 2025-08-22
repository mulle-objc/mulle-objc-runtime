// Test for Chapter 7: Object Creation with Initialization
// Tests creating objects and initializing their ivars

#import "include.h"
#include <stdio.h>
#include <string.h>

@interface Person
{
    int _age;
    char *_name;
}
- (int)age;
- (void)setAge:(int)age;
- (char *)name;
- (void)setName:(char *)name;
@end

@implementation Person
- (int)age { return _age; }
- (void)setAge:(int)age { _age = age; }
- (char *)name { return _name; }
- (void)setName:(char *)name { _name = name; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *object;
    struct _mulle_objc_ivar *age_ivar, *name_ivar;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("Person")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    
    // Initialize age
    age_ivar = mulle_objc_infraclass_search_ivar(cls, 
                mulle_objc_ivarid_from_string("_age"));
    if (age_ivar) {
        *(int *)((char *)object + age_ivar->offset) = 25;
        printf("Age initialized: %d\n", 
               *(int *)((char *)object + age_ivar->offset));
    }
    
    // Initialize name
    name_ivar = mulle_objc_infraclass_search_ivar(cls, 
                mulle_objc_ivarid_from_string("_name"));
    if (name_ivar) {
        *(char **)((char *)object + name_ivar->offset) = "Alice";
        printf("Name initialized: %s\n", 
               *(char **)((char *)object + name_ivar->offset));
    }
    
    printf("Person object created: %p\n", object);
    
    return 0;
}