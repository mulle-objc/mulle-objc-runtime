# Chapter 6: Protocol System

Protocols in mulle-objc are like C interfaces - they define method signatures without implementations. From a C perspective, they're structs containing lists of required and optional methods that classes can conform to.

## Protocol Declaration and Definition

Protocols are declared using `@protocol` syntax and can contain method declarations, property requirements, and other protocol conformances.

```objective-c
@protocol Drawable
- (void)draw;
@property (readonly) CGRect bounds;
@end

@protocol Resizable <Drawable>
- (void)resize:(CGSize)size;
@end
```

## Protocol Lookup

Protocols are registered in the universe and can be looked up by unique protocol ID. Functions with `_nofail` suffix raise exceptions if the requested protocol is not found.

```c
struct _mulle_objc_protocol *protocol;
protocol = _mulle_objc_universe_lookup_protocol_nofail(
    universe, 
    mulle_objc_protocolid_from_string("Drawable")
);
// No NULL check needed - _nofail guarantees success or exception
printf("Protocol: %s\n", _mulle_objc_protocol_get_name(protocol));
```

## Protocol Conformance

Check if a class conforms to a protocol:

```c
struct _mulle_objc_infraclass *cls;
mulle_objc_protocolid_t protocol_id;

cls = mulle_objc_universe_lookup_infraclass_nofail(
    universe, 
    mulle_objc_classid_from_string("Circle")
);
protocol_id = mulle_objc_protocolid_from_string("Drawable");

if (_mulle_objc_infraclass_conformsto_protocolid(cls, protocol_id))
{
    printf("Circle conforms to Drawable\n");
}
```



## Protocol Inheritance

Protocols can inherit from other protocols, creating protocol hierarchies:

```objective-c
@protocol BaseProtocol
- (void)baseMethod;
@end

@protocol ExtendedProtocol <BaseProtocol>
- (void)extendedMethod;
@end
```

## Optional vs Required Methods

Protocols distinguish between required and optional methods:

```objective-c
@protocol OptionalMethods
@required
- (void)mustImplement;
@optional
- (void)canImplement;
@end
```

## Protocol Enumeration

Walk through all protocols in the universe:

```c
static mulle_objc_walkcommand_t print_protocol(struct _mulle_objc_protocol *protocol,
                                               struct _mulle_objc_universe *universe,
                                               void *userinfo)
{
    printf("Protocol: %s\n", _mulle_objc_protocol_get_name(protocol));
    return mulle_objc_walk_ok;
}

_mulle_objc_universe_walk_protocols(universe, 0, print_protocol, NULL);
```

## Protocol Conformance Checking

The runtime provides functions to check protocol conformance at runtime:

```c
// Check if class conforms to protocol
int conforms = _mulle_objc_infraclass_conformsto_protocolid(cls, protocol_id);

// Check if instance conforms to protocol
int conforms = _mulle_objc_object_conformsto_protocolid(obj, protocol_id);
```

## Common Patterns

### Protocol-Based Design

```objective-c
// Define protocol
@protocol DataSource
- (int)numberOfItems;
- (id)itemAtIndex:(int)index;
@end

// Use in class interface
@interface TableView
@property (assign) id<DataSource> dataSource;
@end
```

### Runtime Protocol Checking

```c
// Check if object implements protocol before calling methods
if (_mulle_objc_object_conformsto_protocolid(obj, 
    mulle_objc_protocolid_from_string("DataSource")))
{
    // Safe to call protocol methods
}
```

### Protocol Composition

```objective-c
// Multiple protocol inheritance
@interface Controller : NSObject <Drawable, Resizable, DataSource>
@end
```

## Integration with Categories

Protocols work seamlessly with categories to add method implementations:

```objective-c
// Add protocol conformance via category
@interface NSObject (ResizableSupport) <Resizable>
- (void)resize:(CGSize)size;
@end
```