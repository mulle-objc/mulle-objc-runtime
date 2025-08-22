// Test for Chapter 6: Basic Protocol Declaration
// Tests basic protocol definition and registration

#import "include.h"
#include <stdio.h>

@protocol BasicProtocol
- (void)basicMethod;
@property int value;
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_protocol *protocol;
    
    universe = mulle_objc_global_get_defaultuniverse();
    protocol = _mulle_objc_universe_lookup_protocol(
        universe, 
        mulle_objc_protocolid_from_string("BasicProtocol")
    );
    
    if (protocol) {
        printf("Protocol defined: %s\n", _mulle_objc_protocol_get_name(protocol));
    }
    
    return 0;
}