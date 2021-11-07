#import "Base.h"

@protocol Proto1;
@protocol Proto2;

@interface Foo1 : Base < Proto1, Proto2>
@end
