static void  *mulle_objc_object_call( void *self, unsigned int _cmd, void *_param);

@interface Foo
@end


@implementation Foo

+ (void) gimmeAFoo:(id) foo
{
}

@end

int  main( void)
{
   [Foo gimmeAFoo:(id) 0];
   return( 0);
}

