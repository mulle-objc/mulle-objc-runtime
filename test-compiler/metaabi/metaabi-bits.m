// metaabi-bits.m
// Verify that the compiler emits the correct rtype/ptype bit values into
// method descriptor.bits for all 9 valid ptype x rtype combinations.
//
// Expected mapping (MulleObjCMetaABIType):
//   0 = VoidPointer  (id / single pointer-sized arg)
//   1 = Void         (void return or no explicit params)
//   2 = ParameterBlock (struct larger than void*)
//
// bits 22-23 = rtype, bits 24-25 = ptype
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

typedef struct { int a; double b; } BigStruct;   // > void* -> ParameterBlock

@interface Foo
+ (instancetype) new;
- (void) dealloc;
// rtype=VoidPointer(0), ptype=VoidPointer(0)
- (id) vp_vp:(id) x;
// rtype=Void(1),        ptype=VoidPointer(0)
- (void) v_vp:(id) x;
// rtype=ParameterBlock(2), ptype=VoidPointer(0)
- (BigStruct) pb_vp:(id) x;
// rtype=VoidPointer(0), ptype=Void(1)
- (id) vp_v;
// rtype=Void(1),        ptype=Void(1)
- (void) v_v;
// rtype=ParameterBlock(2), ptype=Void(1)
- (BigStruct) pb_v;
// rtype=VoidPointer(0), ptype=ParameterBlock(2)
- (id) vp_pb:(int) a b:(double) b;
// rtype=Void(1),        ptype=ParameterBlock(2)
- (void) v_pb:(int) a b:(double) b;
// rtype=ParameterBlock(2), ptype=ParameterBlock(2)
- (BigStruct) pb_pb:(int) a b:(double) b;
@end

@implementation Foo
+ (instancetype) new
{
   return _mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self);
}
- (void) dealloc
{
   mulle_objc_instance_free( self);
}
- (id)        vp_vp:(id) x           { return x; }
- (void)      v_vp:(id) x            { (void) x; }
- (BigStruct) pb_vp:(id) x           { BigStruct r = {0,0}; return r; }
- (id)        vp_v                   { return 0; }
- (void)      v_v                    { }
- (BigStruct) pb_v                   { BigStruct r = {0,0}; return r; }
- (id)        vp_pb:(int) a b:(double) b { return 0; }
- (void)      v_pb:(int) a b:(double) b  { }
- (BigStruct) pb_pb:(int) a b:(double) b { BigStruct r = {a, b}; return r; }
@end


static void  print_bits( struct _mulle_objc_infraclass *cls, const char *name, SEL sel)
{
   struct _mulle_objc_method   *method;
   uint32_t                     bits;
   unsigned int                 rtype, ptype;

   method = mulle_objc_class_defaultsearch_method(
               _mulle_objc_infraclass_as_class( cls),
               (mulle_objc_methodid_t) sel);
   if( ! method)
   {
      printf( "%-20s  NOT FOUND\n", name);
      return;
   }

   bits  = method->descriptor.bits;
   rtype = _mulle_objc_method_bits_get_metaabi_rtype( bits);
   ptype = _mulle_objc_method_bits_get_metaabi_ptype( bits);

   printf( "%-20s  rtype=%u  ptype=%u\n", name, rtype, ptype);
}


int  main( void)
{
   Foo                          *foo;
   struct _mulle_objc_infraclass  *cls;

   foo = [Foo new];
   cls = mulle_objc_object_get_infraclass( foo);

   print_bits( cls, "vp_vp",  @selector( vp_vp:));
   print_bits( cls, "v_vp",   @selector( v_vp:));
   print_bits( cls, "pb_vp",  @selector( pb_vp:));

   print_bits( cls, "vp_v",   @selector( vp_v));
   print_bits( cls, "v_v",    @selector( v_v));
   print_bits( cls, "pb_v",   @selector( pb_v));

   print_bits( cls, "vp_pb",  @selector( vp_pb:b:));
   print_bits( cls, "v_pb",   @selector( v_pb:b:));
   print_bits( cls, "pb_pb",  @selector( pb_pb:b:));

   [foo dealloc];

   return( 0);
}
