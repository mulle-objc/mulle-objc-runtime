#include <mulle-objc-runtime/mulle-objc-runtime.h>


typedef struct
{
   NSUInteger   state;
   id          *itemsPtr;
   NSUInteger   *mutationsPtr;
   NSUInteger   extra[5];
} NSFastEnumerationState;


@protocol NSFastEnumeration

- (NSUInteger) countByEnumeratingWithState:(NSFastEnumerationState *) rover
                                  objects:(id *) buffer
                                    count:(NSUInteger) len;
- (NSUInteger) count;

@end


@interface Bar <NSFastEnumeration>
@end


@implementation Bar

- (void) print
{
   printf("Bar\n");
}


- (NSUInteger) countByEnumeratingWithState:(NSFastEnumerationState *) rover
                                   objects:(id *) buffer
                                     count:(NSUInteger) len
{
   id          *sentinel;
   NSUInteger   remain;

   remain = 20 - rover->state;
   if( ! remain)
      return( 0);

   if( remain < len)
      len = remain;

   rover->state   += len;
   rover->itemsPtr = buffer;

   sentinel = &buffer[ len];
   while( buffer < sentinel)
      *buffer++ = self;

   rover->mutationsPtr = &rover->extra[ 4];

   return( len);
}

- (NSUInteger) count
{
   return( 20);
}

@end


@interface Foo  : Bar
@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}

- (void) dealloc
{
   _mulle_objc_instance_free( self);
}

@end


int   main( void)
{
   Foo   *foo;
   Bar   *bar;

   foo = [Foo new];
   for( bar in foo)
      [bar print];
   [foo dealloc];

   return( 0);
}
