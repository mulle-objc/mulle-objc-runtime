#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// multi-arg selector alias
typedef struct objc_object *id;

@interface Foo
- (void)setObject:(id)obj forKey:(id)key;
- (void)setObject:(id)obj forKey:(id)key atIndex:(NSUInteger)i;
- (void)putObject:(id)obj forKey:(id)key;
- (void)putObject:(id)obj forKey:(id)key atIndex:(NSUInteger)i;
@end

@implementation Foo
- (void)setObject:(id)obj forKey:(id)key {}
- (void)setObject:(id)obj forKey:(id)key atIndex:(NSUInteger)i {}
@method_implementation -putObject:forKey: = -setObject:forKey:;
@method_implementation -putObject:forKey:atIndex: = -setObject:forKey:atIndex:;
@end

int main(void) { return 0; }
