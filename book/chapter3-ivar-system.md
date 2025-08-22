# Chapter 3: Ivar System

The ivar system provides access to instance variables within Objective-C objects. In mulle-objc, instance variables are stored as metadata within the class structure, providing type information and offset calculations for direct memory access.

## 3.1 Ivar Access

Instance variables in Objective-C are equivalent to struct members in C. The runtime provides functions to discover ivars by unique ivar ID, enumerate all ivars in a class, and parse their metadata for type information.

### Ivar Lookup

The primary function for ivar lookup is `mulle_objc_infraclass_search_ivar()`, which returns an ivar descriptor. This function takes an infraclass and an ivar ID, returning a `struct _mulle_objc_ivar` structure.

```c
struct _mulle_objc_ivar   *ivar;

ivar = mulle_objc_infraclass_search_ivar(infraclass, "age");
if (ivar)
{
    // Ivar found
    const char *ivar_name = mulle_objc_ivar_get_name(ivar);
    const char *type_encoding = mulle_objc_ivar_get_signature(ivar);
    int offset = mulle_objc_ivar_get_offset(ivar);
}
```

### Ivar Enumeration

To enumerate all ivars in a class, use `_mulle_objc_infraclass_walk_ivars()`. This function walks through all ivars in the class:

```c
static mulle_objc_walkcommand_t print_ivar(struct _mulle_objc_infraclass *infra, 
                                           struct _mulle_objc_ivar *ivar, 
                                           void *info)
{
    printf("ivar: %s %s offset:%d\n",
           mulle_objc_ivar_get_name(ivar),
           mulle_objc_ivar_get_signature(ivar),
           mulle_objc_ivar_get_offset(ivar));
    return(MULLE_OBJC_WALK_CONTINUE);
}

_mulle_objc_infraclass_walk_ivars(infraclass, MULLE_OBJC_INHERIT_SELF, print_ivar, NULL);
```

### Ivar Attributes

Ivar attributes are encoded as strings that describe type information. The format follows Apple's Objective-C encoding scheme, with the encodings supported by mulle-objc-runtime:

- `c` - char
- `i` - int
- `s` - short
- `l` - long
- `q` - long long
- `C` - unsigned char
- `I` - unsigned int
- `S` - unsigned short
- `L` - unsigned long
- `Q` - unsigned long long
- `f` - float
- `d` - double
- `D` - long double
- `B` - BOOL
- `v` - void
- `*` - char pointer (C string)
- `@` - object (retained)
- `~` - object (assigned)
- `#` - class
- `:` - selector
- `^` - generic pointer
- `?` - undefined/function pointer
- `[array_type]` - array
- `{struct_name=fields...}` - struct
- `(union_name=fields...)` - union
- `b`number - bitfield
- `!` - vector

Example attribute strings:
```
"i"           // int
"f"           // float
"*"           // char*
"@"           // id (retained object)
"~"           // id (assigned object)
"{CGRect={CGPoint=dd}{CGSize=dd}}"  // CGRect struct
```

### Direct Ivar Access

ivars can be accessed directly using offset calculations from the ivar descriptor:

```c
struct _mulle_objc_ivar   *ivar;
intptr_t                  offset;

ivar = mulle_objc_infraclass_search_ivar(infraclass, "counter");
if (ivar)
{
    offset = mulle_objc_ivar_get_offset(ivar);
    int *counter_ptr = (int *)((char *)object + offset);
    *counter_ptr = 42;  // Direct ivar assignment
}
```

### Ivar Type Safety

The runtime provides type checking through the encoding system. While direct access is possible, using the encoding from ivars provides type safety:

```c
// Safe approach using runtime type checking
struct _mulle_objc_ivar *ivar = mulle_objc_infraclass_search_ivar(infraclass, "value");
const char *encoding = mulle_objc_ivar_get_signature(ivar);
if (encoding[0] == 'i')
{
    // Confirmed: this ivar is an int
    intptr_t offset = mulle_objc_ivar_get_offset(ivar);
    int *int_ptr = (int *)((char *)object + offset);
}
```

### Practical Example: Ivar Inspection

Here's a complete example demonstrating ivar discovery and inspection:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

struct Person
{
    struct _mulle_objc_object   _base;
    int                         age;
    char                        *name;
    double                      height;
};

static mulle_objc_walkcommand_t inspect_ivar(struct _mulle_objc_infraclass *infra, 
                                             struct _mulle_objc_ivar *ivar, 
                                             void *info)
{
    const char *name = mulle_objc_ivar_get_name(ivar);
    const char *encoding = mulle_objc_ivar_get_signature(ivar);
    int offset = mulle_objc_ivar_get_offset(ivar);
    
    printf("ivar: %-10s encoding: %-20s offset: %d\n", 
           name, encoding, offset);
    return(MULLE_OBJC_WALK_CONTINUE);
}

int  main()
{
    struct _mulle_objc_infraclass   *person_class;
    
    person_class = mulle_objc_get_infraclass_from_string("Person");
    if (!person_class)
        return 1;
        
    printf("Person class ivars:\n");
    _mulle_objc_infraclass_walk_ivars(person_class, MULLE_OBJC_INHERIT_SELF, inspect_ivar, NULL);
    
    return 0;
}
```

### Ivar Offset Calculation

The offset returned by `mulle_objc_ivar_get_offset()` is calculated from the start of the object structure. This allows direct memory access:

```c
// Calculate ivar location
void *object_start = object;
intptr_t ivar_offset = mulle_objc_ivar_get_offset(ivar);
void *ivar_location = (char *)object_start + ivar_offset;

// Type-specific access based on encoding
const char *encoding = mulle_objc_ivar_get_signature(ivar);
switch (encoding[0])
{
    case 'i': *(int *)ivar_location = 123; break;
    case 'f': *(float *)ivar_location = 3.14f; break;
    case 'd': *(double *)ivar_location = 2.718; break;
    case '@': *(id *)ivar_location = some_object; break;
}
```

The ivar system provides the foundation for property access and object introspection, enabling both runtime reflection and direct memory manipulation for performance-critical code paths.