// Test for Chapter 6: Protocol Methods
// Tests protocol with required and optional methods

#import "include.h"
#include <stdio.h>

@protocol MethodProtocol
@required
- (void)requiredMethod;
@property int requiredProperty;

@optional
- (void)optionalMethod;
@property int optionalProperty;
@end

@interface MethodClass
@end

@implementation MethodClass
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_protocol *protocol;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    protocol = _mulle_objc_universe_lookup_protocol(
        universe, 
        mulle_objc_protocolid_from_string("MethodProtocol")
    );
    
    if (protocol) {
        printf("Protocol: %s\n", _mulle_objc_protocol_get_name(protocol));
    }
    
    return 0;
}