#include <mulle_objc_runtime/mulle_objc_runtime.h>


// no support for the package keyword
@interface Foo
{
@package
	int  foo;
}
@end


int   main( void)
{
   return( 0);
}
