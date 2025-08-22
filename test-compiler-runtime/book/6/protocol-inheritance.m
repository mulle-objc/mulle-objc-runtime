// Test for Chapter 6: Protocol Inheritance
// Tests protocol inheritance and method inheritance

#import "include.h"
#include <stdio.h>

@protocol BaseProtocol
- (void)baseMethod;
@end

@protocol ExtendedProtocol <BaseProtocol>
- (void)extendedMethod;
@end

@interface TestClass
@end

@implementation TestClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_protocol *base;
    struct _mulle_objc_protocol *extended;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    base = _mulle_objc_universe_lookup_protocol(
        universe, 
        mulle_objc_protocolid_from_string("BaseProtocol")
    );
    
    extended = _mulle_objc_universe_lookup_protocol(
        universe, 
        mulle_objc_protocolid_from_string("ExtendedProtocol")
    );
    
    if (base) {
        printf("Base protocol: %s\n", _mulle_objc_protocol_get_name(base));
    }
    
    if (extended) {
        printf("Extended protocol: %s\n", _mulle_objc_protocol_get_name(extended));
    }
    
    return 0;
}