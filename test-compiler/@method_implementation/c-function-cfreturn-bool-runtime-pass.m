#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// Runtime test: verify that a C function returning BOOL actually delivers
// the correct value through the MetaABI machinery.

static int call_count;

static BOOL return_yes(id self, SEL _cmd, void *_param) { call_count++; return YES; }
static BOOL return_no(id self, SEL _cmd, void *_param)  { call_count++; return NO;  }

@interface Foo
- (BOOL) yesMethod:(void *) p;
- (BOOL) noMethod:(void *) p;
@end

@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}

@method_implementation -yesMethod: = return_yes;
@method_implementation -noMethod:  = return_no;
@end

int main(void)
{
   Foo   *foo;
   BOOL   result;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   result = [foo yesMethod:NULL];
   printf( "yes=%d\n", (int) result);

   result = [foo noMethod:NULL];
   printf( "no=%d\n", (int) result);

   printf( "calls=%d\n", call_count);

   mulle_objc_instance_free( foo);
   return( result != NO ? 1 : 0);
}
