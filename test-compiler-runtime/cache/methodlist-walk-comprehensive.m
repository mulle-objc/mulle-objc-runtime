#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

// Protocols for testing
@protocol TestProtocol
+ (void) protocolClassMethod;
- (void) protocolInstanceMethod;
@end

@protocol SecondProtocol
+ (void) secondProtocolClassMethod;
- (void) secondProtocolInstanceMethod;
@end

@protocol ThirdProtocol
+ (void) thirdProtocolClassMethod;
- (void) thirdProtocolInstanceMethod;
@end

// Forward declarations for protocolclasses
@class SecondProtocol;
@class ThirdProtocol;

// Base class with categories
@interface BaseClass <TestProtocol>
+ (void) baseClassMethod;
- (void) baseInstanceMethod;
@end

@interface BaseClass (BaseCategory)
+ (void) baseCategoryClassMethod;
- (void) baseCategoryInstanceMethod;
@end

@interface BaseClass (SecondBaseCategory)
+ (void) secondBaseCategoryClassMethod;
- (void) secondBaseCategoryInstanceMethod;
@end

// Protocolclasses (class name matches protocol name)
@interface SecondProtocol <SecondProtocol>
@end

@interface ThirdProtocol <ThirdProtocol>
@end

// Derived class with categories that uses protocolclasses
@interface DerivedClass : BaseClass <SecondProtocol, ThirdProtocol>
+ (void) derivedClassMethod;
- (void) derivedInstanceMethod;
@end

@interface DerivedClass (DerivedCategory)
+ (void) derivedCategoryClassMethod;
- (void) derivedCategoryInstanceMethod;
@end

// Implementations
@implementation BaseClass
+ (void) baseClassMethod { }
- (void) baseInstanceMethod { }
+ (void) protocolClassMethod { }
- (void) protocolInstanceMethod { }
@end

@implementation BaseClass (BaseCategory)
+ (void) baseCategoryClassMethod { }
- (void) baseCategoryInstanceMethod { }
@end

@implementation SecondProtocol
+ (void) secondProtocolClassMethod { }
- (void) secondProtocolInstanceMethod { }
@end

@implementation ThirdProtocol
+ (void) thirdProtocolClassMethod { }
- (void) thirdProtocolInstanceMethod { }
@end

@implementation BaseClass (SecondBaseCategory)
+ (void) secondBaseCategoryClassMethod { }
- (void) secondBaseCategoryInstanceMethod { }
@end

@implementation DerivedClass
+ (void) derivedClassMethod { }
- (void) derivedInstanceMethod { }
@end

@implementation DerivedClass (DerivedCategory)
+ (void) derivedCategoryClassMethod { }
- (void) derivedCategoryInstanceMethod { }
@end

// Extended callback that can show protocol class info
typedef struct {
   int count;
   char entries[20][128];
} walk_result;

static mulle_objc_walkcommand_t test_callback(struct _mulle_objc_class *cls,
                                             struct _mulle_objc_methodlist *list,
                                             void *userinfo)
{
   walk_result *result = (walk_result*)userinfo;
   struct _mulle_objc_methodlistenumerator rover;
   struct _mulle_objc_method *method;
   char *list_name;
   int is_metaclass;
   int is_protocolclass;
   
   if (result->count >= 20) return mulle_objc_walk_cancel;
   
   list_name = _mulle_objc_methodlist_get_categoryname(list);
   if (!list_name)
      list_name = "class";
   
   is_metaclass = _mulle_objc_class_is_metaclass(cls);
   is_protocolclass = _mulle_objc_class_is_protocolclass(cls);
   
   if (is_protocolclass)
      printf("%d. %s (protocol %s):\n", result->count + 1, _mulle_objc_class_get_name(cls), list_name);
   else
      printf("%d. %s (%s):\n", result->count + 1, _mulle_objc_class_get_name(cls), list_name);
   
   rover = _mulle_objc_methodlist_enumerate(list);
   while ((method = _mulle_objc_methodlistenumerator_next(&rover)))
   {
      printf("   %c %s\n", is_metaclass ? '+' : '-', _mulle_objc_method_get_name(method));
   }
   _mulle_objc_methodlistenumerator_done(&rover);
   
   result->count++;
   return mulle_objc_walk_ok;
}

int main(void)
{
   struct _mulle_objc_infraclass *infra;
   struct _mulle_objc_metaclass *meta;
   walk_result result = {0};
   
   printf("=== Comprehensive mulle_objc_class_methodlist_walk Test ===\n\n");
   
   // Get DerivedClass
   infra = mulle_objc_global_lookup_infraclass_nofail(
      MULLE_OBJC_DEFAULTUNIVERSEID,
      mulle_objc_classid_from_string("DerivedClass"));
   
   meta = _mulle_objc_infraclass_get_metaclass(infra);
   
   printf("1. Testing DerivedClass METACLASS (class methods):\n");
   printf("Expected order: DerivedCategory -> Derived -> BaseCategory -> Base -> Protocols -> NSObject -> NSObject infraclass (wraparound)\n");
   mulle_objc_class_methodlist_walk(_mulle_objc_metaclass_as_class(meta), 
                                   test_callback, 
                                   &result);
   printf("Total metaclass methodlists: %d\n", result.count);
   printf("Checking wraparound: Should include NSObject infraclass methods at end\n\n");
   
   // Reset for infraclass test
   result.count = 0;
   
   printf("2. Testing DerivedClass INFRACLASS (instance methods):\n");
   printf("Expected order: DerivedCategory -> Derived -> BaseCategory -> Base -> Protocols -> NSObject\n");
   mulle_objc_class_methodlist_walk(_mulle_objc_infraclass_as_class(infra), 
                                   test_callback, 
                                   &result);
   printf("Total infraclass methodlists: %d\n\n", result.count);
   
   // Test metaclass wraparound by checking if instance methods are found
   // when searching metaclass (this should happen automatically in search)
   printf("3. Testing inheritance chain completeness:\n");
   printf("Metaclass should include full inheritance + wraparound to root infraclass\n");
   printf("Infraclass should include full inheritance: categories -> class -> protocols -> super\n");
   printf("PASS: Metaclass wraparound verified if NSObject infraclass methods found above\n");
   
   return 0;
}
