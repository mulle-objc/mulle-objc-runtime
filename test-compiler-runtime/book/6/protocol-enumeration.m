// Test for Chapter 6: Protocol Enumeration
// Tests walking through all protocols in the universe

#import "include.h"
#include <stdio.h>

@protocol ProtocolA
- (void)methodA;
@end

@protocol ProtocolB
- (void)methodB;
@end

@protocol ProtocolC
- (void)methodC;
@end

static mulle_objc_walkcommand_t print_protocol(struct _mulle_objc_universe *universe,
                                               struct _mulle_objc_protocol *protocol,
                                               void *userinfo)
{
    printf("Protocol: %s\n", _mulle_objc_protocol_get_name(protocol));
    return mulle_objc_walk_ok;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    printf("All protocols:\n");
    _mulle_objc_universe_walk_protocols(universe, print_protocol, NULL);
    
    return 0;
}