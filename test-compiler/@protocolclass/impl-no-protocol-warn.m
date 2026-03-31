#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocolimplementation without @protocolclass declaration warns
@protocolimplementation Unknown
- (void)doSomething {}
@end

int main(void) { return 0; }
