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