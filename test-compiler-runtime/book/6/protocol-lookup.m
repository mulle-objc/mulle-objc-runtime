// Test for Chapter 6: Protocol Lookup
// Tests runtime protocol lookup and examination

#import "include.h"
#include <stdio.h>

@protocol Drawable
- (void)draw;
@property (readonly) int bounds;
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_protocol *protocol;
    
    universe = mulle_objc_global_get_defaultuniverse();
    protocol = _mulle_objc_universe_lookup_protocol(
        universe, 
        mulle_objc_protocolid_from_string("Drawable")
    );
    
    if (protocol) {
        printf("Found protocol: %s\n", _mulle_objc_protocol_get_name(protocol));
        
    }
    
    return 0;
}