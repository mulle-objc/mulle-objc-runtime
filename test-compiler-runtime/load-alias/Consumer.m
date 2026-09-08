#import "Consumer.h"

#pragma clang diagnostic ignored "-Wmulle-method-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation Consumer

//
// +baseValue is NOT here -- it is supplied by the Consumer( Cat) category in
// Consumer+Cat.m, which loads later. The runtime cannot resolve this alias at
// Consumer's load time and must abort loudly (method-not-found), rather than
// leaving a dispatchable method with a garbage implementation.
//
@method_implementation +aliasToCat = +baseValue;

@end
