static void  *mulle_objc_object_call( void *self, unsigned int _cmd, void *_param);

@interface Foo
@end


@implementation Foo

+ (char *) gimmeAFoo:(id) Foo  // named like the class!
{
   return( "VfL Bochum 1848\n");
}

@end


//
// this test triggers a path in the compiler where we had problems in
// 21.0.6.0
// The problem was fixed with adding a isHidden parameter to
// ActOnParmDeclaration
//
int  main( void)
{
   [Foo gimmeAFoo:(id) 0];
   return( 0);
}
