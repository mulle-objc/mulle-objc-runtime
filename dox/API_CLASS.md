# Class

The class is the central data structure, that manages objects. Since the
fundamental structure of the Mac OS X runtime and the mulle-objc runtime is
very similiar, it makes sense to read this introduction to classes:
[What is a meta-class in Objective-C?](https://www.cocoawithlove.com/2010/01/what-is-meta-class-in-objective-c.html)

Nevertheless here are some definitions:

Name         | Definition
-------------|-------------------------------------------
class        | A class is one of a classpair
classpair    | A classpair consists of an infraclass and a metaclass.
infraclass   | Often just synonym with class, this holds the methods for instances (like -dealloc)
metaclass    | This holds the methods for classes (like +alloc)
superclass   | The class a class inherits from (`@interface Foo : Bar  // Bar is the superclass`)
protocol     | A protocol is a unique id, that tags a classpair. You can ask a class if it conforms to a protocol (has this tag)
category     | An extension of a classpair, that adds methods to it


## Functions

All functions allow a NULL argument for `cls`. The return value will be 0,
except where indicated otherwise.


### `mulle_objc_class_add_methodlist`

```
int   mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                       struct _mulle_objc_methodlist *list)
```

Add methodlist `list` to `cls`. The methodlist isn't copied, so be sure that is sticks around for the lifetime of the runtime. (Check out `mulle_objc_runtime_unfailing_add_gift`)


### `mulle_objc_classpair_add_protocolid`

```
void   mulle_objc_classpair_add_protocolid( struct _mulle_objc_class *cls,
mulle_objc_protocolid_t protocolid)
```

Add `protocolid` to class `cls`. The protocolid should be added to the infraclass and to the metaclass using two calls to `mulle_objc_class_add_protocol`. The function does not check for duplicates.



### `mulle_objc_infraclass_is_protocol_class`

```
int  mulle_objc_infraclass_is_protocol_class( struct _mulle_objc_infraclass *cls);
```

A class is a protocol class if (and only if)

1. It is a root class (has no superclass)
2. It has one and only one protocol. That protocolid must be identical to the
classid (which means both names are the same)
3. It has no instance variables

Example:

```
@protocol Foo
@end

@implementation Foo < Foo>
@end
```

### `mulle_objc_class_get_superclass`

```
struct _mulle_objc_class   *mulle_objc_class_get_superclass( struct _mulle_objc_class *cls)
```

Return the superclass of `cls`. If `cls` is a root class, it will return NULL.


### `mulle_objc_class_get_classid`

```
mulle_objc_classid_t   mulle_objc_class_get_classid( struct _mulle_objc_class *cls)
```

Return the classid of `cls`. This is the unique identifying hash of the class.
It can also be computed by calling `mulle_objc_classid_from_string` on the
name of the class.


### `mulle_objc_class_get_name`

```
char   *mulle_objc_class_get_name( struct _mulle_objc_class *cls)
```

Get the name of the class.


### `mulle_objc_class_get_runtime`

```
struct _mulle_objc_runtime   *mulle_objc_class_get_runtime( struct _mulle_objc_class *cls)
```

Get the runtime `cls` is a part of. It is possible to have multiple runtimes,
and calling `mulle_objc_class_get_runtime` on `cls` is the preferred way of
getting access to the runtime pointer.


### `mulle_objc_class_get_instance_size`

```
size_t   mulle_objc_class_get_instance_size( struct _mulle_objc_class *cls)
```

Returns the size of `cls` instances. The returned value is different if you
query the infraclass or the metaclass. Returns `(size_t) -1` if `cls` is NULL.
You usually want to ask the infraclass:

A class with no instance variables with return 0.

Example:

```
cls = mulle_objc_object_get_class( obj);
printf( "Instance size: %ld\n", (long) mulle_objc_class_get_instance_size( cls));
```


### `mulle_objc_class_get_inheritance`

```
unsigned int   mulle_objc_class_get_inheritance( struct _mulle_objc_class *cls)
```

Returns the way **methods** are inherited from superclasses and categories.
The returned value is a combination of the following bits:

```
enum
{
   MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS          = 0x01,
   MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES          = 0x02,
   MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS           = 0x04,
   MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES = 0x08
};
```
Returns ~0 (all unsigned int bits set) if `cls` is NULL.


### `mulle_objc_class_get_forwardmethod`

```
struct _mulle_objc_method   *mulle_objc_class_get_forwardmethod( struct _mulle_objc_class *cls)
```

Returns the method used for forwarding. Forwarding happens when you message
an object with a method it doesn't support. In this case the forward method
is called.


### `mulle_objc_class_is_infraclass`

```
int   mulle_objc_class_is_infraclass( struct _mulle_objc_class *cls)
```

Returns 1 if `cls` is an infraclass. The infraclass is what you get, when you
call `mulle_objc_object_get_class`.


### `mulle_objc_class_is_metaclass`

```
int   mulle_objc_class_is_metaclass( struct _mulle_objc_class *cls)
```

Returns 1 if `cls` is a metaclass.


### `mulle_objc_class_get_infraclass`

```
struct _mulle_objc_class   *mulle_objc_class_get_infraclass( struct _mulle_objc_class *cls)
```

Given any class of a classpair, returns the infraclass.


### `mulle_objc_class_get_metaclass`

```
struct _mulle_objc_class   *mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
```

Given any class of a classpair, returns the metaclass.


### `mulle_objc_class_conformsto_protocol`

```
int   mulle_objc_class_conformsto_protocol( struct _mulle_objc_class *cls, mulle_objc_protocolid_t protocolid)
```

Returns 1 if `cls` conforms to a protocol with `protocolid`. This can be asked
off the metaclass and the infraclass with the same result.  You can get the
`propertyid` for a property by its name using
`mulle_objc_protocolid_from_string`.


### `mulle_objc_class_search_property`

```
struct _mulle_objc_property  *mulle_objc_class_search_property(
                                 struct _mulle_objc_class *cls,
                                 mulle_objc_propertyid_t propertyid);
```

Search for a property with `propertyid` in `cls`. You can get the `propertyid`
for a property by its name using `mulle_objc_protocolid_from_string`. The
returned property can then be used for further introspection.


### `mulle_objc_class_search_ivar`

```
struct _mulle_objc_ivar  *mulle_objc_class_search_ivar( struct _mulle_objc_class *cls,
                                                        mulle_objc_ivarid_t ivarid);
```

Search for an instance variable with `ivarid` in `cls`. You can get the `ivarid`
for a ivar by its name using `mulle_objc_ivarid_from_string`. The
returned property can then be used for further introspection.


