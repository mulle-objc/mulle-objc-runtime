#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stddef.h>
#include "defs.h"


@class C;
@protocol C

@optional
+ (Class) class;
- (void) instanceC;
+ (void) classC;

@end

@interface C <C>
@end

@implementation C

+ (Class) class
{
   return( self);
}

- (void) instanceC
{
}

+ (void) classC
{
}

@end


@class D;
@protocol D

@optional
+ (Class) class;
- (void) instanceD;
+ (void) classD;

@end

@interface D <D>
@end


@implementation D

+ (Class) class
{
   return( self);
}

- (void) instanceD
{
}

+ (void) classD
{
}

@end



@interface A

+ (Class) class;
- (void) instanceA;
+ (void) classA;

@end


@implementation A

+ (Class) class
{
   return( self);
}

- (void) instanceA
{
}

+ (void) classA
{
}

@end


@interface B  : A < C, D>
@end


@implementation B

- (void) instanceA // override
{
}

+ (void) classA   // override
{
}

- (void) instanceB
{
}

+ (void) classB
{
}


@end


@interface E < C, D>

+ (Class) class;
- (void) instanceE;
+ (void) classE;

@end


@implementation E

+ (Class) class
{
   return( self);
}

- (void) instanceE
{
}

+ (void) classE
{
}

@end


static int  test_infra( struct _mulle_objc_infraclass *infra,
                        mulle_objc_methodid_t sel)
{
   struct _mulle_objc_method           *method;
   struct _mulle_objc_searcharguments  arguments;
   mulle_objc_implementation_t         imp1;
   CORE_ADDR                           imp2;
   struct gdbarch                      gdbarch;

   _mulle_objc_searcharguments_defaultinit( &arguments, sel);

   method = mulle_objc_class_search_method( _mulle_objc_infraclass_as_class( infra),
                                            &arguments,
                                            _mulle_objc_infraclass_get_inheritance( infra),
                                            NULL);
   if( method)
      imp1 = _mulle_objc_method_get_implementation( method);
   else
      imp1 = 0;

   gdbarch_init( &gdbarch);
   imp2 = objc_find_implementation_from_class( &gdbarch,
                                               (CORE_ADDR) _mulle_objc_infraclass_as_class( infra),
                                               (CORE_ADDR) sel,
                                               _mulle_objc_infraclass_get_inheritance( infra),
                                               0);
   if( (CORE_ADDR) imp1 != imp2)
      return( -1);

   return( 0);
}


static int  test_meta( struct _mulle_objc_metaclass *meta,
                       mulle_objc_methodid_t sel)
{
   struct _mulle_objc_method           *method;
   struct _mulle_objc_searcharguments  arguments;
   mulle_objc_implementation_t         imp1;
   CORE_ADDR                           imp2;
   struct gdbarch                      gdbarch;

   _mulle_objc_searcharguments_defaultinit( &arguments, sel);

   method = mulle_objc_class_search_method( _mulle_objc_metaclass_as_class( meta),
                                            &arguments,
                                            _mulle_objc_metaclass_get_inheritance( meta),
                                            NULL);
   if( method)
      imp1 = _mulle_objc_method_get_implementation( method);
   else
      imp1 = 0;

   gdbarch_init( &gdbarch);
   imp2 = objc_find_implementation_from_class( &gdbarch,
                                               (CORE_ADDR) _mulle_objc_metaclass_as_class( meta),
                                               (CORE_ADDR) sel,
                                               _mulle_objc_metaclass_get_inheritance( meta),
                                               0);
   if( (CORE_ADDR) imp1 != imp2)
      return( -1);

   return( 0);
}



static int  test_infra_super( struct _mulle_objc_infraclass *infra,
                               mulle_objc_methodid_t sel)
{
   struct _mulle_objc_method           *method;
   struct _mulle_objc_searcharguments  arguments;
   mulle_objc_implementation_t         imp1;
   CORE_ADDR                           imp2;
   struct gdbarch                      gdbarch;

   _mulle_objc_searcharguments_superinit( &arguments,
                                          sel,
                                          _mulle_objc_infraclass_get_classid( infra));

   method = mulle_objc_class_search_method( _mulle_objc_infraclass_as_class( infra),
                                            &arguments,
                                            _mulle_objc_infraclass_get_inheritance( infra),
                                            NULL);
   if( method)
      imp1 = _mulle_objc_method_get_implementation( method);
   else
      imp1 = 0;

   gdbarch_init( &gdbarch);
   imp2 = objc_find_implementation_from_class( &gdbarch,
                                               (CORE_ADDR) _mulle_objc_infraclass_as_class( infra),
                                               (CORE_ADDR) sel,
                                               _mulle_objc_infraclass_get_inheritance( infra),
                                               _mulle_objc_infraclass_get_classid( infra));
   if( (CORE_ADDR) imp1 != imp2)
      return( -1);

   return( 0);
}


static int  test_meta_super( struct _mulle_objc_metaclass *meta,
                             mulle_objc_methodid_t sel)
{
   struct _mulle_objc_method           *method;
   struct _mulle_objc_searcharguments  arguments;
   mulle_objc_implementation_t         imp1;
   CORE_ADDR                           imp2;
   struct gdbarch                      gdbarch;

   _mulle_objc_searcharguments_superinit( &arguments,
                                          sel,
                                          _mulle_objc_metaclass_get_classid( meta));

   method = mulle_objc_class_search_method( _mulle_objc_metaclass_as_class( meta),
                                            &arguments,
                                            _mulle_objc_metaclass_get_inheritance( meta),
                                            NULL);
   if( method)
      imp1 = _mulle_objc_method_get_implementation( method);
   else
      imp1 = 0;

   gdbarch_init( &gdbarch);
   imp2 = objc_find_implementation_from_class( &gdbarch,
                                               (CORE_ADDR) _mulle_objc_metaclass_as_class( meta),
                                               (CORE_ADDR) sel,
                                               _mulle_objc_metaclass_get_inheritance( meta),
                                               _mulle_objc_metaclass_get_classid( meta));
   if( (CORE_ADDR) imp1 != imp2)
      return( -1);

   return( 0);
}



static int   test( void)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   int                             i, j;
   Class                           classes[ 5];
   mulle_objc_methodid_t           selectors[ 10] =
   {
      @selector( instanceA),
      @selector( instanceB),
      @selector( instanceC),
      @selector( instanceD),
      @selector( instanceE),
      @selector( classA),
      @selector( classB),
      @selector( classC),
      @selector( classD),
      @selector( classE)
   };

   classes[ 0] = [A class];
   classes[ 1] = [B class];
   classes[ 2] = [C class];
   classes[ 3] = [D class];
   classes[ 4] = [E class];

   for( i = 0; i < 5; i++)
   {
      infra = (struct _mulle_objc_infraclass *) classes[ i];
      meta  = _mulle_objc_infraclass_get_metaclass( infra);
      for( j = 0; j < 10; j++)
      {
         fprintf( stderr, "%d.%d: infra\n", i, j);
         if( test_infra( infra, selectors[ j]))
            return( 1);
         fprintf( stderr, "%d.%d: meta\n", i, j);
         if( test_meta( meta, selectors[ j]))
            return( 1);
         fprintf( stderr, "%d.%d: infra super\n", i, j);
         if( test_infra_super( infra, selectors[ j]))
            return( 1);
         fprintf( stderr, "%d.%d: meta super\n", i, j);
         if( test_meta_super( meta, selectors[ j]))
            return( 1);
      }
   }
   return( 0);
}


static int  test3( void)
{
   struct _mulle_objc_infraclass       *infra;
   struct _mulle_objc_metaclass        *meta;
   struct _mulle_objc_method           *method;
   struct _mulle_objc_searcharguments  arguments;
   mulle_objc_implementation_t         imp1;
   CORE_ADDR                           imp2;
   struct gdbarch                      gdbarch;

   infra = (struct _mulle_objc_infraclass *) [E class];
   meta  = _mulle_objc_infraclass_get_metaclass( infra);

   _mulle_objc_searcharguments_superinit( &arguments,
                                          @selector( instanceC),
                                          _mulle_objc_metaclass_get_classid( meta));

   method = mulle_objc_class_search_method( _mulle_objc_metaclass_as_class( meta),
                                            &arguments,
                                            _mulle_objc_metaclass_get_inheritance( meta),
                                            NULL);
   if( ! method)
      return( -1);
   imp1 = _mulle_objc_method_get_implementation( method);

   gdbarch_init( &gdbarch);
   imp2 = objc_find_implementation_from_class( &gdbarch,
                                               (CORE_ADDR) _mulle_objc_metaclass_as_class( meta),
                                               (CORE_ADDR) @selector( instanceC),
                                               _mulle_objc_metaclass_get_inheritance( meta),
                                               _mulle_objc_metaclass_get_classid( meta));
   if( (CORE_ADDR) imp1 != imp2)
      return( -2);
   return( 0);
}


int   main( int argc, char *argv[])
{
   // these values are used in objc_lang.c for offset calculation
   // they are CPU/ABI dependent
   struct _mulle_objc_infraclass       *infra;
   struct _mulle_objc_method           *method;
   struct _mulle_objc_searcharguments  arguments;
   mulle_objc_implementation_t         imp1;
   CORE_ADDR                           imp2;
   struct gdbarch                      gdbarch;

   fprintf( stderr, "pair.infraclass      = %ld\n", (long) offsetof( struct _mulle_objc_classpair, infraclass));
   fprintf( stderr, "pair.metaclass       = %ld\n", (long) offsetof( struct _mulle_objc_classpair, metaclass));
   fprintf( stderr, "pair.protocolclasses = %ld\n", (long) offsetof( struct _mulle_objc_classpair, protocolclasses));

   infra = (struct _mulle_objc_infraclass *) [A class];

   fprintf( stderr, "A infraclass      = %p\n", infra);
   fprintf( stderr, "A metaclass       = %p\n", _mulle_objc_infraclass_get_metaclass( infra));
   fprintf( stderr, "A pair            = %p\n", _mulle_objc_infraclass_get_classpair( infra));
   fprintf( stderr, "A protocolclasses = %p\n", _mulle_objc_infraclass_get_classpair( infra)->protocolclasses.storage.storage);

   infra = (struct _mulle_objc_infraclass *) [B class];

   fprintf( stderr, "B infraclass      = %p\n", infra);
   fprintf( stderr, "B metaclass       = %p\n", _mulle_objc_infraclass_get_metaclass( infra));
   fprintf( stderr, "B pair            = %p\n", _mulle_objc_infraclass_get_classpair( infra));
   fprintf( stderr, "B protocolclasses = %p\n", _mulle_objc_infraclass_get_classpair( infra)->protocolclasses.storage.storage);

   infra = (struct _mulle_objc_infraclass *) [C class];

   fprintf( stderr, "C infraclass      = %p\n", infra);
   fprintf( stderr, "C metaclass       = %p\n", _mulle_objc_infraclass_get_metaclass( infra));
   fprintf( stderr, "C pair            = %p\n", _mulle_objc_infraclass_get_classpair( infra));
   fprintf( stderr, "C protocolclasses = %p\n", _mulle_objc_infraclass_get_classpair( infra)->protocolclasses.storage.storage);


   infra = (struct _mulle_objc_infraclass *) [D class];

   fprintf( stderr, "D infraclass      = %p\n", infra);
   fprintf( stderr, "D metaclass       = %p\n", _mulle_objc_infraclass_get_metaclass( infra));
   fprintf( stderr, "D pair            = %p\n", _mulle_objc_infraclass_get_classpair( infra));
   fprintf( stderr, "D protocolclasses = %p\n", _mulle_objc_infraclass_get_classpair( infra)->protocolclasses.storage.storage);

   infra = (struct _mulle_objc_infraclass *) [E class];

   fprintf( stderr, "E infraclass      = %p\n", infra);
   fprintf( stderr, "E metaclass       = %p\n", _mulle_objc_infraclass_get_metaclass( infra));
   fprintf( stderr, "E pair            = %p\n", _mulle_objc_infraclass_get_classpair( infra));
   fprintf( stderr, "E protocolclasses = %p\n", _mulle_objc_infraclass_get_classpair( infra)->protocolclasses.storage.storage);

   if( test())
      return( 1);

   return( 0);
}


