#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Root
@end
@implementation Root
@end


@class Proto;
@protocol Proto
@end
@interface Proto < Proto>
@end
@implementation Proto
@end


@interface A : Root < Proto>
@end
@implementation A
@end


@interface A( B)
@end
@implementation A( B)
@end


int   main( int argc, char *argv[])
{
   // we happy if we don't crash
   mulle_objc_htmldump_universe_to_tmp();
   mulle_objc_dotdump_universe_to_tmp();
   return( 0);
}
