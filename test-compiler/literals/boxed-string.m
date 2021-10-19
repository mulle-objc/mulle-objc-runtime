#undef __MULLE_OBJC_TPS__
#define __MULLE_OBJC_NO_TPS__

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>
#include <string.h>

@interface NSString

+ (id) stringWithUTF8String:(char *) s;

@end


@implementation NSString

+ (id) stringWithUTF8String:(char *) s
{
   printf( "%s\n", s);
   return( self);
}

- (int) length
{
   return( 4);
}

@end


int  main( int argc, char *argv[])
{
   id   foo;

   //
   // this has a tendency to be compiled away by the compiler if a
   // static C-string is used. So we use input provided by .args file
   //
   foo = @( argv[ 1]);
   // just be happy that it compiles :)
   printf( "%d\n", [foo length]);
   printf( "%d\n", strlen( argv[ 1]));
   return( 0);
}

