#include <mulle-objc-runtime/mulle-objc-runtime.h>
// C function with wrong return type: int instead of void * or void
typedef struct objc_selector *SEL;
typedef struct objc_object   *id;

int bad_c_function(id self, SEL _cmd, void *);

@interface Foo
- (void *)doSomething;
@end

@implementation Foo
@method_implementation -doSomething = bad_c_function;
@end
