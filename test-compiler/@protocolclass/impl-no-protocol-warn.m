#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol_implementation without @protocol_class declaration warns
@protocol_implementation Unknown
- (void)doSomething {}
@end

int main(void) { return 0; }
