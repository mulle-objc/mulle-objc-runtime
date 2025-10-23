# Chapter 5: Property System

Properties in mulle-objc are metadata constructs that establish the relationship between instance variables and their accessor methods. From a C perspective, they define struct member offsets along with function signatures for the generated getter and setter implementations.

## Property Declaration and Access

Properties are declared in the `@interface` section and generate concrete runtime data: ivar entries, method descriptors, and accessor implementations. The compiler creates these components based on the property declaration.

```objective-c
@interface Person : NSObject
@property (nonatomic, strong) NSString *name;
@property (nonatomic, assign) int age;
@end

@implementation Person
@synthesize name = _name;
@synthesize age = _age;
@end
```

## Property Lookup

Properties are stored in the class's property list and can be looked up by unique property ID:

```c
struct _mulle_objc_property *prop;
prop = _mulle_objc_infraclass_search_property(person_class, 
                                       mulle_objc_propertyid_from_string("name"));
if (prop)
{
    printf("Property: %s\n", _mulle_objc_property_get_name(prop));
}
```

## Property Attributes

Property attributes control memory management and access patterns:

```objective-c
@property (nonatomic, strong) NSString *title;    // Retains object
@property (nonatomic, weak) id delegate;          // Weak reference
@property (nonatomic, assign) int counter;        // Direct assignment
@property (nonatomic, copy) NSString *text;       // Makes copy
@property (readonly) NSString *identifier;        // No setter
```

## Property Enumeration

Walk through all properties of a class:

```c
static mulle_objc_walkcommand_t print_property(struct _mulle_objc_property *property,
                                               struct _mulle_objc_infraclass *infra,
                                               void *userinfo)
{
    printf("Property: %s (%s)\n", 
           _mulle_objc_property_get_name(property),
           _mulle_objc_property_get_signature(property));
    return mulle_objc_walk_ok;
}

_mulle_objc_infraclass_walk_properties(person_class, 0, print_property, NULL);
```

## Ivar Access via Properties

Properties provide type-safe access to instance variables:

```objective-c
Person *p = [[Person alloc] init];
p.name = @"Alice";          // Calls setName:
NSString *n = p.name;       // Calls name
```

## Dynamic Properties

Properties can be resolved at runtime without ivars:

```objective-c
@interface DynamicObject : NSObject
@property (nonatomic, strong) id dynamicValue;
@end

@implementation DynamicObject
@dynamic dynamicValue;

- (id)dynamicValue
{
    return objc_getAssociatedObject(self, "dynamicValue");
}

- (void)setDynamicValue:(id)value
{
    objc_setAssociatedObject(self, "dynamicValue", value, OBJC_ASSOCIATION_RETAIN);
}
@end
```

## Property Metadata

Access property metadata programmatically:

```c
struct _mulle_objc_property *prop = _mulle_objc_infraclass_search_property(cls, 
                                                             name_id);
if (prop)
{
    const char *attrs = _mulle_objc_property_get_signature(prop);
    printf("Attributes: %s\n", attrs);
    
    const char *type = _mulle_objc_property_get_signature(prop);
    printf("Type: %s\n", type);
}
```

## Property Categories

Properties can be added via categories:

```objective-c
@interface Person (Extended)
@property (nonatomic, strong) NSString *email;
@end

@implementation Person (Extended)
- (NSString *)email
{
    return objc_getAssociatedObject(self, "email");
}

- (void)setEmail:(NSString *)email
{
    objc_setAssociatedObject(self, "email", email, OBJC_ASSOCIATION_RETAIN);
}
@end
```

## Common Patterns

### Property Validation

```objective-c
- (void)setAge:(int)age
{
    if (age >= 0)
        _age = age;
}
```

### Lazy Loading

```objective-c
- (NSArray *)items
{
    if (!_items)
        _items = [[NSArray alloc] init];
    return _items;
}
```

### Key-Value Compliance

```objective-c
// Automatic for synthesized properties
[person setValue:@"Bob" forKey:@"name"];
NSString *name = [person valueForKey:@"name"];
```

## Property Introspection and Getter/Setter Methods

The runtime provides complete introspection capabilities for properties, including access to their getter and setter method IDs:

### Property Getter/Setter Introspection

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void demonstrate_property_introspection(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_property *property;
    mulle_objc_propertyid_t property_id;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string("Person")
    );
    
    // Find specific property
    property_id = mulle_objc_propertyid_from_string("name");
    property = mulle_objc_class_search_property(&cls->base, property_id);
    
    if (property)
    {
        const char *name = mulle_objc_property_get_name(property);
        const char *signature = mulle_objc_property_get_signature(property);
        mulle_objc_propertyid_t propertyid = mulle_objc_property_get_propertyid(property);
        mulle_objc_methodid_t getter = mulle_objc_property_get_getter(property);
        mulle_objc_methodid_t setter = mulle_objc_property_get_setter(property);
        
        printf("Property: %s\n", name);
        printf("  ID: 0x%08x\n", propertyid);
        printf("  Signature: %s\n", signature);
        printf("  Getter method ID: 0x%08x\n", getter);
        printf("  Setter method ID: 0x%08x\n", setter);
    }
}
```

### Property List Enumeration

Walk through all properties of a class using the property list walker:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Callback for property enumeration
static int print_property(struct _mulle_objc_property *property,
                          struct _mulle_objc_class *cls,
                          void *userinfo)
{
    const char *prefix = (const char *)userinfo;
    
    printf("%sProperty: %-15s Type: %-10s Getter: 0x%08x Setter: 0x%08x\n",
            prefix,
            mulle_objc_property_get_name(property),
            mulle_objc_property_get_signature(property),
            mulle_objc_property_get_getter(property),
            mulle_objc_property_get_setter(property));
    
    return 0; // Continue walking
}

void enumerate_class_properties(const char *class_name)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_propertylist *propertylist;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string(class_name)
    );
    
    propertylist = mulle_objc_class_get_properties(&cls->base);
    if (propertylist)
    {
        printf("Properties in %s:\n", class_name);
        mulle_objc_propertylist_walk(propertylist, print_property, 
                                     &cls->base, "  ");
    }
}
```

### Property List Enumerator Usage

For more control over property enumeration:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void enumerate_properties_with_enumerator(const char *class_name)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_propertylist *propertylist;
    struct _mulle_objc_propertylistenumerator enumerator;
    struct _mulle_objc_property *property;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string(class_name)
    );
    
    propertylist = mulle_objc_class_get_properties(&cls->base);
    if (propertylist)
    {
        enumerator = mulle_objc_propertylist_enumerate(propertylist);
        
        printf("Properties in %s:\n", class_name);
        while ((property = mulle_objc_propertylistenumerator_next(&enumerator)))
        {
            printf("  %s: %s\n",
                   mulle_objc_property_get_name(property),
                   mulle_objc_property_get_signature(property));
        }
        
        mulle_objc_propertylistenumerator_done(&enumerator);
    }
}
```

## Property Signature vs Ivar Signature

Property signatures and ivar signatures serve different purposes and may contain different type information:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void demonstrate_signature_differences(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_property *property;
    struct _mulle_objc_ivar *ivar;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string("Person")
    );
    
    // Compare property signature vs ivar signature
    property = mulle_objc_class_search_property(
        &cls->base,
        mulle_objc_propertyid_from_string("name")
    );
    
    ivar = mulle_objc_class_search_ivar(
        &cls->base,
        mulle_objc_ivarid_from_string("_name")
    );
    
    if (property && ivar)
    {
        printf("Property signature: %s\n", 
               mulle_objc_property_get_signature(property));
        printf("Ivar signature:     %s\n", 
               mulle_objc_ivar_get_signature(ivar));
        
        // Property signature may include attributes like 'R' (readonly)
        // while ivar signature is just the type encoding
    }
}
```

## Key APIs Summary

| Function | Purpose |
|----------|---------|
| `mulle_objc_class_search_property()` | Search for property by ID |
| `mulle_objc_property_get_name()` | Get property name |
| `mulle_objc_property_get_signature()` | Get property signature |
| `mulle_objc_property_get_propertyid()` | Get unique property ID |
| `mulle_objc_property_get_getter()` | Get getter method ID |
| `mulle_objc_property_get_setter()` | Get setter method ID |
| `mulle_objc_propertylist_walk()` | Walk property list with callback |
| `mulle_objc_propertylist_enumerate()` | Get property list enumerator |
| `mulle_objc_propertylistenumerator_next()` | Get next property from enumerator |
| `mulle_objc_propertylistenumerator_done()` | Clean up enumerator |