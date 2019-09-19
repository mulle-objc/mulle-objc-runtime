#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


// this tries to replicate a problem I had with EOAttribute

@interface Foo
{
    id                 _name;
    struct
    {
        unsigned int   a:1;
        unsigned int   b:1;
        unsigned int   c:1;
        unsigned int   bits:18;
        unsigned int   d:1;       // addition
        unsigned int   e:2;
        unsigned int   f:8;
   } _flags;
}

+ (id) alloc;
- (void) dealloc;

@end


@implementation Foo


+ (id) alloc
{
   Foo  *obj;

   obj = _mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self);
   obj->_flags.bits = 0x10;
   return( obj);
}


- (unsigned int) bits
{
   return( self->_flags.bits);
}


- (void) dealloc
{
   _mulle_objc_object_free( self);
}



static int   isBitSet( Foo *self, unsigned int bit)
{
   int            flag;
   unsigned int   bits;

   bits = self->_flags.bits;
   flag = (bits & bit) ? 1 : 0;
   return( flag);
}

@end


int   main( int argc, char *argv[])
{
   Foo     *foo;
   int     rval;

   foo   = [Foo alloc];
   rval  = ! isBitSet( foo, 0x10); // NO is fail
   rval |= isBitSet( foo, 0x8);    // YES is fail
   rval |= [foo bits] != 0x10;     // YES is fail
   [foo dealloc];

   if( rval)
      fprintf( stderr, "FAILED\n");

   return( rval);
}
