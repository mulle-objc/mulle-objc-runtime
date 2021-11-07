#import "Base.h"

@class ProtoClass1;
@protocol ProtoClass1;
@class ProtoClass2;
@protocol ProtoClass2;

@interface Foo3 : Base < ProtoClass1, ProtoClass2>
@end
