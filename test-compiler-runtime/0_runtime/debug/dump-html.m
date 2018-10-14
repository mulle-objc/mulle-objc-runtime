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
   struct _mulle_objc_universe  *universe;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);
   mulle_objc_universe_htmldump_to_directory( universe, ".");
   mulle_objc_universe_dotdump_to_directory( universe, ".");
   return( 0);
}
