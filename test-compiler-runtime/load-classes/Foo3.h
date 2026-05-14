#import "Base.h"

@mixin ProtoClass1;
@mixin ProtoClass2;

@interface Foo3 : Base < ProtoClass1, ProtoClass2>
@end
