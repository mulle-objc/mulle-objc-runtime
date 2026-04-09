#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// C function alias: mulle metaabi signature
// zero-arg method: (id, SEL) -> void * **WRONG**
// with-arg method: (id, SEL, void *) -> void *

typedef struct objc_selector *SEL;
typedef struct objc_object   *id;

void *my_zero_arg(id self, SEL _cmd) { return( NULL); }
void *my_with_arg(id self, SEL _cmd, void *_param) { return( NULL); }

@interface Foo
- (void *)doSomething;
- (void *)doSomethingWith:(id)x;
@end

@implementation Foo
@method_implementation -doSomething = my_zero_arg;
@method_implementation -doSomethingWith: = my_with_arg;
@end

int main(void) { return 0; }
