#import "Base.h"

@protocol_class ProtoClass1;
@protocol_class ProtoClass2;

@interface Foo3 : Base < ProtoClass1, ProtoClass2>
@end
