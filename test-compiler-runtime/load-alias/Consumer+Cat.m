#import "Consumer.h"

//
// This category supplies the alias target +baseValue, but in a separate
// translation unit that is deliberately loaded AFTER Consumer (see
// CMakeLists.txt source order). A category is not a load dependency of its
// class, so this arrives too late for the alias in Consumer.m to resolve.
//
@interface Consumer( Cat)
@end

@implementation Consumer( Cat)

+ (id) baseValue
{
   return( self);
}

@end
