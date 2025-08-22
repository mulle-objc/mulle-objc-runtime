// Test for Chapter 2: Enumerating All Classes
// Tests walking through all loaded classes

#import "include.h"
#include <stdio.h>

@interface TestClass
@end
@implementation TestClass
@end

static mulle_objc_walkcommand_t print_class_name(struct _mulle_objc_universe *universe, void *cls, enum mulle_objc_walkpointertype_t type, char *key, void *parent, void *userinfo)
{
    if (type == mulle_objc_walkpointer_is_infraclass || type == mulle_objc_walkpointer_is_metaclass)
        printf("%s\n", _mulle_objc_infraclass_get_name((struct _mulle_objc_infraclass *)cls));
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    printf("All loaded classes:\n");
    mulle_objc_universe_walk_classes(universe, print_class_name, NULL);
    
    return 0;
}