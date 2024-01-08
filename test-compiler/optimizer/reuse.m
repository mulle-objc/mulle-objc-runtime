#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@implementation Foo

+ (void) check:(void *) a
         param:(void *) param
{
   printf( "%s\n", param == _param ? "Reused" : "Not reused");
}


+ (void)  test1:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   // reuse OK
   x = (uintptr_t) _param;
   [self check:a
         param:(void *) x];
}


+ (void)  test2:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse not OK
   x = (uintptr_t) _param;
   [self check:&a
         param:(void *) x];
}


+ (void)  test3:(id) a
               :(id) b
{
   uintptr_t   x;
   id          *alias;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse OK
   x     = (uintptr_t) _param;
   alias = &a;
   [self check:a
         param:(void *) x];
}


+ (void)  test4:(id) a
               :(id) b
{
   uintptr_t   x;
   id          *alias;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse not OK
   x = (uintptr_t) _param;
   alias = &a;
   [self check:alias
         param:(void *) x];
}


+ (void)  test5:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);

   // reuse not OK
   x = (uintptr_t) _param;
   [self check:a
         param:(void *) x];

   // reuse OK
   [self check:a
         param:(void *) x];
}


+ (void)  test6:(id) a
               :(id) b
{
   uintptr_t   x;
   id          *alias;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse OK
   x     = (uintptr_t) _param;
   [self check:a
         param:(void *) x];

   // reuse OK
   [self check:self
         param:(void *) x];
}


+ (void)  test7:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse not
   x = (uintptr_t) _param;
label:
   [self check:a
         param:(void *) x];
   if( x == 0)
      goto label;
}


+ (void)  test8:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse not OK
   x = (uintptr_t) _param;
   do
   {
      [self check:a
            param:(void *) x];
   }
   while( 0);
}


+ (void)  test9:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse not OK
   x = (uintptr_t) _param;
   while( x)
   {
      [self check:a
            param:(void *) x];
      x = 0;
   }
}


+ (void)  testa:(id) a
               :(id) b
{
   uintptr_t   x;

   printf( "%s\n", __PRETTY_FUNCTION__);
   // reuse not OK
   x = (uintptr_t) _param;
   for( ;x;)
   {
      [self check:a
            param:(void *) x];
      x = 0;
   }
}

@end


int   main( void)
{
   [Foo test1:(id) 0x18 :(id) 0x48];
   [Foo test2:(id) 0x18 :(id) 0x48];
   [Foo test3:(id) 0x18 :(id) 0x48];
   [Foo test4:(id) 0x18 :(id) 0x48];
   [Foo test5:(id) 0x18 :(id) 0x48];
   [Foo test6:(id) 0x18 :(id) 0x48];
   [Foo test7:(id) 0x18 :(id) 0x48];
   [Foo test8:(id) 0x18 :(id) 0x48];
   [Foo test9:(id) 0x18 :(id) 0x48];
   [Foo testa:(id) 0x18 :(id) 0x48];

   return( 0);
}
