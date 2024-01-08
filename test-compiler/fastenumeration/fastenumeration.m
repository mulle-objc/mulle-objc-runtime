#include <mulle-objc-runtime/mulle-objc-runtime.h>


typedef struct
{
   uintptr_t   state;
   id              *itemsPtr;
   uintptr_t   *mutationsPtr;
   uintptr_t   extra[5];
} NSFastEnumerationState;


@protocol NSFastEnumeration

- (uintptr_t) countByEnumeratingWithState:(NSFastEnumerationState *) rover
                                  objects:(id *) buffer
                                    count:(uintptr_t) len;
- (uintptr_t) count;

@end


@interface Bar <NSFastEnumeration>
@end


@implementation Bar

- (void) print
{
   printf("Bar\n");
}


- (uintptr_t) countByEnumeratingWithState:(NSFastEnumerationState *) rover
                                  objects:(id *) buffer
                                    count:(uintptr_t) len
{
   id          *sentinel;
   uintptr_t   remain;

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

- (uintptr_t) count
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
