#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif



static mulle_objc_implementation_t
   search_super_implementation( id self,
                                mulle_objc_methodid_t methodid,
                                mulle_objc_classid_t classid)
{
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_class             *cls;
   struct _mulle_objc_method            *method;
   mulle_objc_implementation_t          imp;
   unsigned int                         inheritance;

   if( ! self)
      return( (mulle_objc_implementation_t) 0);

   cls         = _mulle_objc_object_get_isa( self);
   inheritance = cls->inheritance;
   search      = mulle_objc_searcharguments_make_super( methodid, classid);
   method      = mulle_objc_class_search_method( cls,
                                                 &search,
                                                 inheritance,
                                                 NULL);
   imp = 0;
   if( method)
      imp = _mulle_objc_method_get_implementation( method);

   return( imp);
}


static void  print_self_and_call_super_if_available( id self,
                         mulle_objc_methodid_t _cmd,
                         mulle_objc_classid_t classid,
                         char *name)
{
   static unsigned int           recursion;
   mulle_objc_implementation_t   imp;

   mulle_printf( "\t%s\n", name);

   imp = search_super_implementation( self, _cmd, classid);
   if( imp)
   {
      if( ++recursion > 10)
      {
         mulle_fprintf( stderr, "**recursion**\n");
         exit(1);
      }
      mulle_objc_implementation_invoke( imp, self, _cmd, self);
      --recursion;
   }
}

//
// MEMO: a mixin may "adopt" another mixin, but only as
//       a protocol (syntactically)
//
@mixin A;
@mixin A
@end

@implementation A
+ (void) print
{
   print_self_and_call_super_if_available( self, _cmd, @selector( A), (char *) __PRETTY_FUNCTION__);
}
@end


@mixin B;
@mixin B <A>
@end

@implementation B
+ (void) print
{
   print_self_and_call_super_if_available( self, _cmd, @selector( B), (char *) __PRETTY_FUNCTION__);
}
@end



@interface C < A, B>
@end


@implementation C
+ (void) print
{
   print_self_and_call_super_if_available( self, _cmd, @selector( C), (char *) __PRETTY_FUNCTION__);
}
@end



@interface D < B>
@end


@implementation D
+ (void) print
{
   print_self_and_call_super_if_available( self, _cmd, @selector( D), (char *) __PRETTY_FUNCTION__);
}
@end


@interface E < A>
@end


@implementation E
+ (void) print
{
   print_self_and_call_super_if_available( self, _cmd, @selector( E), (char *) __PRETTY_FUNCTION__);
}
@end


// MEMO: THIS SEEMS TO WORK NOW!
@interface F < B, A>
@end


@implementation F
+ (void) print
{
   print_self_and_call_super_if_available( self, _cmd, @selector( F), (char *) __PRETTY_FUNCTION__);
}
@end


int   main()
{
   mulle_printf( "C:\n");
   [C print];
   mulle_printf( "D:\n");
   [D print];
   mulle_printf( "E:\n");
   [E print];
   mulle_printf( "F:\n");
   [F print];
   return( 0);
}
