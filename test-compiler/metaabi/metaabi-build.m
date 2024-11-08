//
//  main.m
//  test-meta-abi
//
//  Created by Nat! on 19.08.24
//  Copyright © 2024 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>
#include <ctype.h>


//
// this test check if we can build up an example metaabi frame with
// mulle-vararg, we should be able to...
//

/***************************************************************************************
# metaABI - struct parameters

A quick introduction to how the metaABI uses alignof and sizeof without the
usual argument promotion to construct a call frame, and how struct parameter
members differ from regular C function promotion. The metaABI's approach allows for more
predictable and controllable parameter passing, especially when dealing with
complex data structures or when interfacing between different languages or
calling conventions.

In C function calls, arguments undergo promotion. For example, `char` and
`short` are promoted to `int`, and `float` is promoted to `double`.
The metaABI instead builds the call frame like the C-compiler would build
a C struct.


``` objc
- (void) a:(char) a
         b:(char) b
         c:(float) c
         d:(struct { char x; int y; }) d
```

On a x64 processors, the information will be passed in [registers](https://cs61.seas.harvard.edu/site/2018/Asm2/)
RDI, RSI, RDX:

```
RDI:
+-------------------------------+
|              self             |  8 bytes (object pointer)
+-------------------------------+

RSI:
+-------------------------------+
|                    _cmd       |  4 bytes (selector constant)
+-------------------------------+  (stored in 8 bytes register)

RDX:
+-------------------------------+
|            _param             |  8 bytes (pointer to metaABI param block)
+-------------------------------+
```

The `_param` block contains the arguments to the method, in metaABI fashion:


```
_param:
+---+---+-------+---------------+  2 bytes for 2 chars
| a | b |       |      c        |  2 bytes alignment for float
+---+---+-------+---------------+  4 bytes float
|d.x|           |     d.y       |
+---+-----------+---------------+  1 byte  (char x from struct)
                                   3 bytes (to struct align int y)
                                   4 bytes (int y from struct)

```
****************/


@interface Foo
@end


@implementation Foo

+ (instancetype) new
{
   id   obj;

   obj = _mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self);
   return( obj);
}

- (void) dealloc
{
   mulle_objc_instance_free( self);
}

- (char *) UTF8String
{
   return( "Foo");
}


- (void) dummy
{
}

// metaABI wise it's known that (a) we get a _param block for a double
// (b) that block is at least 5 void * wide
// (c) we can reuse that block if we don't care about its contents anymore
// (d) a return value will be written into _param on return, but that's of
//     no concern here.

- (void) call:(id) obj
     selector:(SEL) sel
  doubleValue:(double) d
{
   mulle_printf( "%s %x %g\n", [obj UTF8String], sel, d);
}


- (void) call:(id) obj
     selector:(SEL) sel
    charValue:(char) c
    charValue:(char) d
{
   c = isprint(c) ? c : '?';
   d = isprint(d) ? d : '?';
   mulle_printf( "%s %x '%c' '%c'\n", [obj UTF8String], sel, c, d);
}

- (void) call:(id) obj, ...
{
   mulle_vararg_list    arguments;
   SEL                  sel;
   char                 c;
   char                 d;

   mulle_vararg_start( arguments, obj);
   sel = mulle_vararg_next_selector( arguments);
   c   = mulle_vararg_next_char( arguments);
   d   = mulle_vararg_next_char( arguments);
   c   = isprint(c) ? c : '?';
   d   = isprint(d) ? d : '?';

   mulle_printf( "%s %x '%c' '%c'\n", [obj UTF8String], sel, c, d);
}

@end


static void   test_double( void)
{
   mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n( sizeof( id) + sizeof( SEL) + sizeof( double))];
   mulle_vararg_list             list;
   mulle_vararg_list             q;
   Foo                           *foo;
   SEL                           sel;

   foo  = [Foo new];
   sel  = @selector( dummy);

   list = mulle_vararg_list_make( buf);
   memset( buf, 0xAD, sizeof(buf));
   //
   // sort of an experiment, this is actually wrong because buf is not
   // guaranteed to be 5 void ptr large which violates the meta abi
   // (but doesn't break it in this instance, as the called method doesn't
   // reuse it)
   // The "double" parameter luckily aligns
   mulle_vararg_copy( q, list);
   mulle_vararg_push_object( q, foo);
   mulle_vararg_push_selector( q, sel);
   mulle_vararg_push_double( q, 18.48);

   mulle_objc_object_call( foo, @selector( call:selector:doubleValue:), buf);

   [foo dealloc];
}


static void   test_char_char( void)
{
   mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n( sizeof( id) + sizeof( SEL) + sizeof( char) + sizeof( char))];
   mulle_vararg_list             list;
   mulle_vararg_list             q;
   Foo                           *foo;
   SEL                           sel;

   foo  = [Foo new];
   sel  = @selector( dummy);

   list = mulle_vararg_list_make( buf);
   memset( buf, 0xAD, sizeof(buf));

   // This will not work, because vararg is pushing chars
   // as ints and the method expects chars in struct layout fashion
   // like the metaABI dictates
   mulle_vararg_copy( q, list);
   mulle_vararg_push_object( q, foo);
   mulle_vararg_push_selector( q, sel);
   mulle_vararg_push_char( q, 'A');
   mulle_vararg_push_char( q, 'B');

   mulle_objc_object_call( foo, @selector( call:selector:charValue:charValue:), buf);

   [foo dealloc];
}



static void   check_char_char( void)
{
   Foo   *foo;

   foo  = [Foo new];

   [foo call:foo
    selector: @selector( dummy)
   charValue:'A'
   charValue:'B'];

   [foo dealloc];
}


static void   check_variadic( void)
{
   Foo   *foo;

   foo  = [Foo new];

   [foo call:foo, @selector( dummy), 'A', 'B'];

   [foo dealloc];
}


int   main( void)
{
   test_char_char();
   test_double();

   check_char_char();
   check_variadic();

   return( 0);
}
