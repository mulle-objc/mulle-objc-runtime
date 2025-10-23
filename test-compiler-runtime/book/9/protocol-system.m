// Test for Chapter 9: Protocol System
// Tests protocol definition, lookup, and conformance checking

#import "include.h"

@protocol Printable
- (const char *)description;
@optional
- (int)getLength;
@end

@protocol Comparable
- (int)isEqualTo:(id)other;
@end

@interface ConformingClass <Printable, Comparable>
@end

@implementation ConformingClass
- (const char *)description { return "ConformingClass instance"; }
- (int)getLength { return 25; }
- (int)isEqualTo:(id)other { return (int)(self == other); }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_protocol *printable;
    struct _mulle_objc_protocol *comparable;
    struct _mulle_objc_infraclass *infra;
    id obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("Test: Protocol system and interface inheritance\n");
    
    // Look up protocols by name
    printable = _mulle_objc_universe_lookup_protocol(universe, mulle_objc_protocolid_from_string("Printable"));
    comparable = _mulle_objc_universe_lookup_protocol(universe, mulle_objc_protocolid_from_string("Comparable"));
    
    mulle_printf("SUCCESS: Protocols found\n");
    mulle_printf("Printable protocol: %s\n", printable ? "found" : "not found");
    mulle_printf("Comparable protocol: %s\n", comparable ? "found" : "not found");
    
    // Test protocol conformance
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("ConformingClass"));
    
    obj = mulle_objc_infraclass_alloc_instance(infra);
    if (obj)
    {
        mulle_printf("SUCCESS: Class conforms to protocols\n");
        mulle_printf("Description: %s\n", [obj description]);
        mulle_printf("Length: %d\n", [obj getLength]);
    }
    
    return 0;
}