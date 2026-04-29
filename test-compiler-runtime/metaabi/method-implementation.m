#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wmulle-method-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

// C function targets
static int call_count;

static id self_nop( id self, SEL sel, void *param)
{
   call_count++;
   return self;
}

static void void_nop( id self, SEL sel, void *param)
{
   call_count++;
}

// tc1: class method C func alias, NOT declared in interface
@interface Foo1
@end
@implementation Foo1
@method_implementation +autorelease = self_nop;
@end

// tc2: class method C func alias, method with param, NOT declared
@interface Foo2
@end
@implementation Foo2
@method_implementation +copyWithZone: = self_nop;
@end

// tc3: instance method C func alias, NOT declared
@interface Foo3
@end
@implementation Foo3
@method_implementation -finalize = void_nop;
@end

// tc4: method-to-method chain, none declared
@interface Foo4
@end
@implementation Foo4
@method_implementation +retain        = self_nop;
@method_implementation +copy          = +retain;
@method_implementation +immutableCopy = +copy;
@end

// tc5: re-aliasing chain — verify all three resolve to same C function at runtime
@interface Foo5
@end
@implementation Foo5
@method_implementation +retain        = self_nop;
@method_implementation +copy          = +retain;           // alias of alias
@method_implementation +immutableCopy = +copy;             // alias of alias of alias
@method_implementation +copyWithZone: = self_nop;          // 1-arg selector, undeclared
@end

int main( void)
{
   // tc1–tc4: just check they compile and run without crashing
   mulle_printf( "compile-ok\n");

   // tc5: verify re-aliasing actually dispatches to self_nop each time
   struct _mulle_objc_universe *universe = mulle_objc_global_get_defaultuniverse();
   struct _mulle_objc_infraclass *cls =
       mulle_objc_universe_lookup_infraclass_nofail( universe,
           mulle_objc_classid_from_string( "Foo5"));

   call_count = 0;
   mulle_objc_object_call( cls, @selector(retain), NULL);
   mulle_printf( "retain: %d\n", call_count);

   mulle_objc_object_call( cls, @selector(copy), NULL);
   mulle_printf( "copy: %d\n", call_count);

   mulle_objc_object_call( cls, @selector(immutableCopy), NULL);
   mulle_printf( "immutableCopy: %d\n", call_count);

   mulle_objc_object_call( cls, @selector(copyWithZone:), NULL);
   mulle_printf( "copyWithZone:: %d\n", call_count);
   return 0;
}
