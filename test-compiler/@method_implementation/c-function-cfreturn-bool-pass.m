#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// C function returning BOOL matching ObjC method's BOOL return type.
// This tests the hasSameUnqualifiedType arm of the return-type check,
// which was the regression case from NSConstantString.

static BOOL NSConstantStringGetData(id self, SEL _cmd, void *_param)
{
   return( YES);
}

@interface Foo
- (BOOL) mulleFastGetASCIIData:(void *) space;
- (BOOL) mulleFastGetUTF8Data:(void *) space;
@end

@implementation Foo
@method_implementation -mulleFastGetASCIIData: = NSConstantStringGetData;
@method_implementation -mulleFastGetUTF8Data:  = NSConstantStringGetData;
@end

int main(void) { return 0; }
