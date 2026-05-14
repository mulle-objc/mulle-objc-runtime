#import "Base.h"

@mixin ProtoClass2;
@protocol Proto1;

@interface Foo2 : Base < ProtoClass2, Proto1>
@end
