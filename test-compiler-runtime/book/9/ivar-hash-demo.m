// Test for ivar offset calculation
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Define a test class
@interface TestClass
{
    int value;
    char *name;
}
@end

@implementation TestClass
@end

int main(void)
{
    struct _mulle_objc_universe   *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_ivar       *ivar;
    mulle_objc_ivarid_t           ivarid;
    int                           offset;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(universe, 
                                                       mulle_objc_classid_from_string("TestClass"));
    
    if (cls)
    {
        ivarid = mulle_objc_ivarid_from_string("isa");
        ivar = mulle_objc_infraclass_search_ivar(cls, ivarid);
        if (ivar)
        {
            offset = mulle_objc_ivar_get_offset(ivar);
            printf("Ivar 'isa' offset: %d\n", offset);
        }
    }
    
    return 0;
}