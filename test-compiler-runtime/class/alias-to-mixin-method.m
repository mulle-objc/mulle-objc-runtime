#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
# pragma message "this test needs mulle-objc"
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wmulle-method-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

//
// Regression test for: a class-local @method_implementation alias whose TARGET
// method is supplied by an adopted @mixin (not by the class itself).
//
// Historically the runtime logged the alias as "Deferred ... not yet loaded"
// while the mixin's method lists were not installed yet, then never retried the
// resolution. method->implementation was left holding the alias methodid (it
// shares a union with `alias`), so sending the alias selector jumped into that
// bogus address -> SIGSEGV.
//
// Controls (already worked, kept here to bracket the bug):
//   - calling the mixin method directly
//   - aliasing a class-LOCAL target (defined on the same class)
//
// NB: the aliasing class must NOT be a root class -- the compiler rejects
// runtime alias resolution in root classes -- so Consumer subclasses Base.
//

static int   base_calls;      // +baseValue provided by the mixin
static int   local_calls;     // +localValue provided by the class itself


// root base class with the minimal plumbing
@interface Base
@end

@implementation Base
+ (void) initialize {}
@end


//
// A mixin that provides the alias TARGET method. This is the crux: the alias
// target lives in the companion (mixin) class, whose method lists may not be
// loaded when the aliasing class is processed.
//
@mixin ProviderMixin;
@mixin ProviderMixin
@end

@implementation ProviderMixin

+ (id) baseValue
{
   base_calls++;
   return( self);
}

@end


//
// Consumer adopts the mixin and aliases the mixin-provided +baseValue under a
// second name +aliasToMixin. It also aliases a class-local +localValue under
// +aliasToLocal as a control.
//
@interface Consumer : Base < ProviderMixin >
@end

@implementation Consumer

+ (id) localValue
{
   local_calls++;
   return( self);
}

// (1) alias -> mixin-provided target  (the crashing case)
@method_implementation +aliasToMixin = +baseValue;

// (2) alias -> class-local target     (control: already works)
@method_implementation +aliasToLocal = +localValue;

@end


int   main( void)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_infraclass   *cls;

   universe = mulle_objc_global_get_defaultuniverse();
   cls      = mulle_objc_universe_lookup_infraclass_nofail(
                 universe,
                 mulle_objc_classid_from_string( "Consumer"));

   // control A: mixin method reachable directly
   base_calls = 0;
   mulle_objc_object_call( cls, @selector( baseValue), NULL);
   mulle_printf( "baseValue: %d\n", base_calls);

   // control B: alias to a class-local target resolves and dispatches
   local_calls = 0;
   mulle_objc_object_call( cls, @selector( aliasToLocal), NULL);
   mulle_printf( "aliasToLocal: %d\n", local_calls);

   // the bug: alias to a mixin-provided target must dispatch into the mixin's
   // +baseValue, not into a garbage IMP.
   base_calls = 0;
   mulle_objc_object_call( cls, @selector( aliasToMixin), NULL);
   mulle_printf( "aliasToMixin: %d\n", base_calls);

   return( 0);
}
