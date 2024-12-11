#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}


- (void) noValue
{
}

- (id) value
{
   return( 0);
}

- (id) notAValue:(id) param
{
   return( param);
}


- (void) setValue:(id) param
{
}


- (id) setNotAValue:(id) param
{
   return( param);
}

- (void) setNotAValue:(id) param
            notAValue:(id) param2
{
}


- (void) setNoValue
{
}


- (void) setvalue
{
}


- (void) addTovalue:(id) Foo
{
}


- (void) removeFromvalue:(id) Foo
{
}



- (void) addToValue:(id) Foo
{
}


- (void) removeFromValue:(id) Foo
{
}


///------------

- (void) mulleNoValue
{
}

- (id) mulleValue
{
   return( 0);
}

- (id) mulleNotAValue:(id) param
{
   return( param);
}


- (void) mulleSetValue:(id) param
{
}


// not a setter, because it returns
- (id) mulleSetNotAValue:(id) param
{
   return( param);
}

- (void) mulleSetNotAValue:(id) param
                 notAValue:(id) param2
{
}


- (void) mulleSetNoValue
{
}



- (void) mulleAddToValue:(id) Foo
{
}


- (void) mulleRemoveFromValue:(id) Foo
{
}

- (void) mulleSetnoValue
{
}



- (void) mulleAddTonalue:(id) Foo
{
}


- (void) mulleRemoveFromvalue:(id) Foo
{
}

//
//   _mulle_objc_methodfamily_alloc       = 1,
//   _mulle_objc_methodfamily_copy        = 2,
//   _mulle_objc_methodfamily_init        = 3,
//   _mulle_objc_methodfamily_mutablecopy = 4,
//   _mulle_objc_methodfamily_new         = 5,
//
//   _mulle_objc_methodfamily_autorelease = 6,
//   _mulle_objc_methodfamily_dealloc     = 7,
//   _mulle_objc_methodfamily_finalize    = 8,
//   _mulle_objc_methodfamily_release     = 9,
//   _mulle_objc_methodfamily_retain      = 10,
//   _mulle_objc_methodfamily_retaincount = 11,
//   _mulle_objc_methodfamily_self        = 12,
//   _mulle_objc_methodfamily_initialize  = 13,
//
//   _mulle_objc_methodfamily_perform     = 14,
//   // methods returning non-void, no args
//   _mulle_objc_methodfamily_getter      = 32,
//   // methods starting with set[A-Z] returning void and one arg
//   _mulle_objc_methodfamily_setter      = 33,
//   _mulle_objc_methodfamily_adder       = 34,
//   _mulle_objc_methodfamily_remover     = 35
+ (id) initialize
{
   return( 0);
}

+ (id) mulleInitialize
{
   return( 0);
}

+ (id) alloc
{
   return( 0);
}

+ (id) mulleAlloc
{
   return( 0);
}

+ (id) new
{
   return( 0);
}

+ (id) mulleNew
{
   return( 0);
}


- (id) mutableCopy
{
   return( 0);
}

- (id) mulleMutableCopy
{
   return( 0);
}


- (id) copy
{
   return( 0);
}

- (id) mulleCopy
{
   return( 0);
}


- (instancetype) init
{
   return( 0);
}

- (instancetype) mulleInit
{
   return( 0);
}


- (instancetype) initWithWhatever:(id) foo
{
   return( 0);
}

- (instancetype) mulleInitWithWhatever:(id) foo
{
   return( 0);
}



- (instancetype) initWithWhatever:(id) foo
                            other:(id) other
{
   return( 0);
}


- (instancetype) mulleInitWithWhatever:(id) foo
                                 other:(id) other
{
   return( 0);
}


- (void) finalize
{
}

- (void) mulleFinalize
{
}


- (void) dealloc
{
}

- (void) mulleDealloc
{
}


- (void) release
{
}

- (void) mulleRelease
{
}


- (instancetype) retain
{
   return( 0);
}

- (instancetype) mulleRetain
{
   return( 0);
}


- (uintptr_t) retainCount
{
   return( 0);
}

- (uintptr_t) mulleRetainCount
{
   return( 0);
}


- (instancetype) self
{
   return( 0);
}

- (instancetype) mulleSelf
{
   return( 0);
}


- (id) performSelector:(SEL) sel
{
   return( 0);
}


@end


static void   print_desc( struct _mulle_objc_descriptor *desc)
{
   printf( "%s = (%d) %s%s%s%s\n", _mulle_objc_descriptor_get_name( desc),
      _mulle_objc_descriptor_get_methodfamily( desc),
      _mulle_objc_descriptor_is_getter_method( desc)  ? " getter"  : "",
      _mulle_objc_descriptor_is_setter_method( desc)  ? " setter"  : "",
      _mulle_objc_descriptor_is_adder_method( desc)   ? " adder"   : "",
      _mulle_objc_descriptor_is_remover_method( desc) ? " remover" : "");

}


static mulle_objc_walkcommand_t  add_descriptor( struct _mulle_objc_method *method,
                                                 struct _mulle_objc_methodlist *list,
                                                 struct _mulle_objc_class *cls,
                                                 void *userinfo)
{
   struct mulle_pointerarray       *array = userinfo;
   struct _mulle_objc_descriptor   *desc;

   desc = _mulle_objc_method_get_descriptor( method);
   mulle_pointerarray_add( array, desc);
   return( mulle_objc_walk_ok);
}


int  main( void)
{
   struct _mulle_objc_infraclass   *infra;;
   struct _mulle_objc_descriptor   *desc;

   mulle_pointerarray_do( descriptors)
   {
      infra = [Foo class];
      _mulle_objc_class_walk_methods( _mulle_objc_infraclass_as_class( infra),
                                      0,
                                      add_descriptor,
                                      descriptors);
      mulle_pointerarray_qsort_r( descriptors, _mulle_objc_descriptor_pointer_compare_r, NULL);
      mulle_pointerarray_for( descriptors, desc)
      {
         print_desc( desc);
      }
   }
   return( 0);
}

