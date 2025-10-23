// Test for Chapter 9: Protocolclass Loading
// Tests loading protocolclasses (protocol-conforming classes) through loadinfo

#include "include.h"

// Define a protocol
@protocol Drawable
- (const char *)draw;
@end

// Define a protocolclass that conforms to Drawable
@interface CircleClass <Drawable>
@end

@implementation CircleClass
- (const char *)draw { return "Drawing a circle"; }
@end

// Define another protocolclass
@interface SquareClass <Drawable>
@end

@implementation SquareClass
- (const char *)draw { return "Drawing a square"; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *circleClass;
    struct _mulle_objc_infraclass *squareClass;
    id circleObj;
    id squareObj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    mulle_printf("Test: Protocolclass loading\n");
    
    // Look up the protocolclasses by name
    circleClass = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("CircleClass"));
    
    squareClass = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("SquareClass"));
    
    if (circleClass && squareClass)
    {
        mulle_printf("SUCCESS: Protocolclasses loaded\n");
        
        // Create instances and test protocol conformance
        circleObj = mulle_objc_infraclass_alloc_instance(circleClass);
        squareObj = mulle_objc_infraclass_alloc_instance(squareClass);
        
        if (circleObj && squareObj)
        {
            mulle_printf("Circle: %s\n", [circleObj draw]);
            mulle_printf("Square: %s\n", [squareObj draw]);
            
            mulle_objc_instance_free(circleObj);
            mulle_objc_instance_free(squareObj);
        }
    }
    else
    {
        mulle_printf("ERROR: Protocolclasses not found\n");
    }
    
    return 0;
}