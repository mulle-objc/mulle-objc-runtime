#include <mulle-objc-runtime/mulle-objc-runtime.h>
// C function with wrong param count
typedef struct objc_selector *SEL;
typedef struct objc_object   *id;

void *bad_too_few(id self);

@interface Foo
- (void *)doSomething;
@end

@implementation Foo
@method_implementation -doSomething = bad_too_few;
@end
