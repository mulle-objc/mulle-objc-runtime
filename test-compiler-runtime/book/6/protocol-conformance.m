// Test for Chapter 6: Protocol Conformance
// Tests checking if classes conform to protocols

#import "include.h"
#include <stdio.h>

@protocol ConformanceProtocol
- (void)protocolMethod;
@property int protocolProperty;
@end

@interface ConformingClass
@end

@implementation ConformingClass
- (void)protocolMethod {
    // Implementation
}
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    mulle_objc_protocolid_t protocol_id;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("ConformingClass")
    );
    
    protocol_id = mulle_objc_protocolid_from_string("ConformanceProtocol");
    
    if (_mulle_objc_infraclass_conformsto_protocolid(cls, protocol_id)) {
        printf("Class conforms to protocol\n");
    } else {
        printf("Class does not conform to protocol\n");
    }
    
    return 0;
}