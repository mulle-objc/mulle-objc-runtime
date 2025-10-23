# Chapter 9: Runtime Loading and Dynamic Code Injection

The mulle-objc runtime provides a complete system for dynamically loading classes, categories, protocols, and other runtime objects at runtime. This chapter covers the loadinfo system, which enables dynamic code injection through structured load information.

## 9.1 Loadinfo Structures and Dynamic Loading

The loadinfo system allows you to define and load runtime objects programmatically using static loadinfo structures that are automatically registered with the runtime at program startup.

### Static Loadinfo Structure

A loadinfo structure contains all the information needed to load runtime objects. You define it as a static structure and register it using the constructor pattern:

```c
// Creating a complete loadinfo structure (25 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

// Define our class structure
struct SimpleClass {
    int value;
};

// Method implementation
static int SimpleClass_getValue(struct SimpleClass *self, 
                               mulle_objc_methodid_t _cmd, void *_params)
{
    return self->value;
}

// Loadinfo structure definition
static struct _mulle_objc_loadinfo load_info = {
    {
        MULLE_OBJC_RUNTIME_LOAD_VERSION,
        MULLE_OBJC_RUNTIME_VERSION,
        0, 0, 0  // flags
    },
    NULL,
    &class_list,
    NULL, NULL, NULL, NULL, NULL, NULL
};

// Constructor for automatic loading
MULLE_C_CONSTRUCTOR(__load)
static void __load()
{
    static int has_loaded = 0;
    if (has_loaded) return;
    has_loaded = 1;
    
    mulle_objc_loadinfo_enqueue_nofail(&load_info);
}
```

### Step 1: Defining a Simple Class

Start with a minimal class definition using loadinfo structures:

```c
// Simple class definition with loadinfo (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Generated uniqueid for "SimpleClass"
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)

// Define the actual struct for our class
struct SimpleClass {
    // Empty class for now
};

// Class definition structure
static struct _mulle_objc_loadclass SimpleClass_loadclass = {
    ___SimpleClass_classid,        // classid
    "SimpleClass",                 // name
    0,                             // flags
    0, NULL, 0,                    // superclass info
    -1, sizeof(struct SimpleClass), // instance size
    NULL, NULL, NULL, NULL, NULL   // ivars, methods, properties
};

static struct _mulle_objc_loadclasslist class_list = {
    1, { &SimpleClass_loadclass }
};
```

### Step 2: Adding Instance Variables

Add ivars to your class definition:

```c
// Class with instance variables (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stddef.h>

// Generated uniqueids
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)
#define ___value___ivarid        MULLE_OBJC_IVARID( 0x40c292ce)

struct SimpleClass {
    int value;
};

// Ivar list definition
static struct _mulle_objc_ivar SimpleClass_ivars[] = {
    {
        { ___value___ivarid, "value", "i" },  // ivar descriptor
        offsetof(struct SimpleClass, value)    // offset
    }
};

static struct _mulle_objc_ivarlist SimpleClass_ivarlist = {
    1, SimpleClass_ivars
};

// Update class definition with ivars
static struct _mulle_objc_loadclass SimpleClass_loadclass = {
    ___SimpleClass_classid, "SimpleClass", 0,
    0, NULL, 0,
    -1, sizeof(struct SimpleClass),
    &SimpleClass_ivarlist, NULL, NULL, NULL, NULL
};
```

### Step 3: Adding Methods

Add method implementations to your class:

```c
// Class with methods (22 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Generated uniqueids
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)
#define ___getValue__methodid    MULLE_OBJC_METHODID( 0x55160ecf)
#define ___setValue___methodid   MULLE_OBJC_METHODID( 0x4ddcb6e4)

struct SimpleClass {
    int value;
};

// Method implementations
static int SimpleClass_getValue(struct SimpleClass *self, 
                               mulle_objc_methodid_t _cmd, void *_params)
{
    return self->value;
}

static void SimpleClass_setValue_(struct SimpleClass *self, 
                                 mulle_objc_methodid_t _cmd, 
                                 struct { int _value; } *_params)
{
    self->value = _params->_value;
}

// Method list definition
static struct _mulle_objc_method SimpleClass_methods[] = {
    { { ___getValue__methodid, "i@:", "getValue", 0 }, 
      (mulle_objc_implementation_t)SimpleClass_getValue },
    { { ___setValue___methodid, "v@:i", "setValue:", 0 }, 
      (mulle_objc_implementation_t)SimpleClass_setValue_ }
};

static struct _mulle_objc_methodlist SimpleClass_methodlist = {
    2, NULL, SimpleClass_methods
};
```

### Step 4: Adding Protocols

Add protocol conformance to your class:

```c
// Class with protocol conformance (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Generated uniqueids
#define ___SimpleClass_classid       MULLE_OBJC_CLASSID( 0x83779e1a)
#define ___ValueProtocol_protocolid  MULLE_OBJC_PROTOCOLID( 0x3f7a8b2c)

// Protocol declaration structure
static struct _mulle_objc_protocol ValueProtocol = {
    ___ValueProtocol_protocolid,
    "ValueProtocol",
    0, NULL, NULL, NULL  // Basic protocol structure
};

// Protocol list for class
static struct _mulle_objc_protocol *SimpleClass_protocols[] = {
    &ValueProtocol
};

static struct _mulle_objc_protocollist SimpleClass_protocollist = {
    1, SimpleClass_protocols
};
```

### Step 4b: Adding Protocolclasses (Protocol-Conforming Classes)

Protocolclasses are classes that conform to specific protocols and can be loaded through the `protocolclassids` field in loadinfo structures. This enables dynamic protocol conformance checking and protocol-oriented programming patterns.

```c
// Protocolclass loading with protocol conformance (25 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Generated uniqueids
#define ___Drawable_protocolid   MULLE_OBJC_PROTOCOLID( 0x7f3a8b4c)
#define ___CircleClass_classid   MULLE_OBJC_CLASSID( 0x9e7b2a1f)
#define ___SquareClass_classid   MULLE_OBJC_CLASSID( 0x3c8f9e2a)
#define ___draw__methodid        MULLE_OBJC_METHODID( 0x2a5b8c7d)

// Protocol definition
static struct _mulle_objc_protocol DrawableProtocol = {
    ___Drawable_protocolid, "Drawable", 0, NULL, NULL, NULL
};

// Circle class implementation
struct CircleClass { int radius; };
static const char *CircleClass_draw(struct CircleClass *self, 
                                   mulle_objc_methodid_t _cmd, void *_params) {
    return "Drawing a circle";
}

// Square class implementation  
struct SquareClass { int side; };
static const char *SquareClass_draw(struct SquareClass *self, 
                                   mulle_objc_methodid_t _cmd, void *_params) {
    return "Drawing a square";
}

// Protocolclass list for Circle
static mulle_objc_classid_t Circle_protocolclassids[] = {
    ___Drawable_protocolid
};

// Protocolclass list for Square
static mulle_objc_classid_t Square_protocolclassids[] = {
    ___Drawable_protocolid
};

// Circle class definition with protocolclass
static struct _mulle_objc_loadclass CircleClass_loadclass = {
    ___CircleClass_classid, "CircleClass", 0,
    0, NULL, 0, -1, sizeof(struct CircleClass),
    NULL, NULL, NULL, NULL, NULL,
    NULL, Circle_protocolclassids  // protocolclassids
};

// Square class definition with protocolclass
static struct _mulle_objc_loadclass SquareClass_loadclass = {
    ___SquareClass_classid, "SquareClass", 0,
    0, NULL, 0, -1, sizeof(struct SquareClass),
    NULL, NULL, NULL, NULL, NULL,
    NULL, Square_protocolclassids  // protocolclassids
};
```

### Step 6: Adding Categories

Add categories to extend existing classes:

```c
// Category extension (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Generated uniqueids
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)
#define ___Print_categoryid      MULLE_OBJC_CATEGORYID( 0x8320d28e)
#define ___print__methodid       MULLE_OBJC_METHODID( 0x6378a881)

// Category method implementation
static void SimpleClass_Print_print(struct SimpleClass *self, 
                                   mulle_objc_methodid_t _cmd, void *_params)
{
    printf("SimpleClass value: %d\n", self->value);
}

// Category method list
static struct _mulle_objc_method Print_methods[] = {
    { { ___print__methodid, "v@:", "print", 0 }, 
      (mulle_objc_implementation_t)SimpleClass_Print_print }
};

static struct _mulle_objc_methodlist Print_methodlist = {
    1, NULL, Print_methods
};

// Category definition
static struct _mulle_objc_loadcategory SimpleClass_Print_category = {
    ___Print_categoryid, "Print",
    ___SimpleClass_classid, "SimpleClass", 0,
    NULL, &Print_methodlist, NULL, NULL
};
```

### Step 7: Adding Properties

Add properties with automatic getter/setter generation:

```c
// Complete class with properties (25 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Generated uniqueids
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)
#define ___value___ivarid        MULLE_OBJC_IVARID( 0x40c292ce)
#define ___getValue__methodid    MULLE_OBJC_METHODID( 0x55160ecf)
#define ___setValue___methodid   MULLE_OBJC_METHODID( 0x4ddcb6e4)

struct SimpleClass {
    int value;
};

// Property definition
static struct _mulle_objc_property SimpleClass_properties[] = {
    {
        { ___value___ivarid, "value", "Ti,N" },  // property descriptor
        ___value___ivarid,    // ivar id
        ___getValue__methodid,   // getter method id
        ___setValue___methodid,  // setter method id
        0
    }
};

static struct _mulle_objc_propertylist SimpleClass_propertylist = {
    1, SimpleClass_properties
};
```

## 9.2 Complete Loadinfo Example

Putting it all together in a working loadinfo structure:

```c
// Complete runtime loading example (30 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

// All generated uniqueids
#define ___SimpleClass_classid   MULLE_OBJC_CLASSID( 0x83779e1a)
#define ___value___ivarid        MULLE_OBJC_IVARID( 0x40c292ce)
#define ___getValue__methodid    MULLE_OBJC_METHODID( 0x55160ecf)
#define ___setValue___methodid   MULLE_OBJC_METHODID( 0x4ddcb6e4)

struct SimpleClass {
    int value;
};

// Complete loadinfo structure with all components
static struct _mulle_objc_loadinfo load_info = {
    { MULLE_OBJC_RUNTIME_LOAD_VERSION, MULLE_OBJC_RUNTIME_VERSION, 0, 0, 0 },
    NULL,
    &class_list,
    &category_list,
    &protocol_list,
    NULL, NULL, NULL, NULL
};

// Constructor for automatic loading
MULLE_C_CONSTRUCTOR(__load)
static void __load()
{
    static int has_loaded = 0;
    if (has_loaded) return;
    has_loaded = 1;
    
    mulle_objc_loadinfo_enqueue_nofail(&load_info);
}

int main(void)
{
    struct _mulle_objc_infraclass *cls;
    
    cls = mulle_objc_global_lookup_infraclass_nofail(
        MULLE_OBJC_DEFAULTUNIVERSEID, ___SimpleClass_classid);
    
    if (cls) {
        printf("SUCCESS: SimpleClass loaded with runtime\n");
        printf("Class: %s\n", _mulle_objc_infraclass_get_name(cls));
        printf("Instance size: %zu bytes\n", 
               _mulle_objc_infraclass_get_instancesize(cls));
    }
    
    return 0;
}
```

## 9.3 Dynamic Loading Patterns

### Loading Patterns

The runtime uses the constructor pattern for automatic loading:

**Automatic Loading (Constructor Pattern)**:
```c
MULLE_C_CONSTRUCTOR(__load)
static void __load() {
    static int has_loaded = 0;
    if (has_loaded) return;
    has_loaded = 1;
    
    mulle_objc_loadinfo_enqueue_nofail(&load_info);
}
```

**Manual Triggering**:
```c
// Manual triggering of loadinfo
void trigger_load() {
    // The loadinfo structure is already defined and will be registered
    // when the constructor runs automatically
    printf("Loadinfo will be processed by runtime\n");
}
```

### Loadinfo Memory Management

Loadinfo structures are defined statically and automatically managed by the runtime:

```c
// Static loadinfo definition - no allocation needed
static struct _mulle_objc_loadinfo load_info = {
    {
        MULLE_OBJC_RUNTIME_LOAD_VERSION,
        MULLE_OBJC_RUNTIME_VERSION,
        0, 0, 0  // flags
    },
    NULL,
    &class_list,
    &category_list,
    &protocol_list,
    NULL, NULL, NULL, NULL
};

// The runtime manages the structure after registration
// No cleanup required by user code
```

## 9.4 Advanced Loading Scenarios

### Loading Multiple Classes

Load multiple classes in a single loadinfo:

```c
// Multiple class loading
static struct _mulle_objc_loadclass *all_classes[] = {
    &SimpleClass_loadclass,
    &ComplexClass_loadclass,
    &UtilityClass_loadclass
};

static struct _mulle_objc_loadclasslist all_class_list = {
    3, all_classes
};
```

### Conditional Loading

Load classes based on runtime conditions:

```c
// Conditional loading based on environment
void conditional_load() {
    if (getenv("ENABLE_DEBUG_CLASSES")) {
        mulle_objc_loadinfo_enqueue_nofail(&debug_load_info);
    }
}
```

### Plugin Architecture

Create a plugin system using loadinfo:

```c
// Plugin loading interface
typedef struct _mulle_objc_loadinfo* (*plugin_loader_t)();

void load_plugin(plugin_loader_t loader) {
    struct _mulle_objc_loadinfo *info = loader();
    if (info) {
        mulle_objc_loadinfo_enqueue_nofail(info);
    }
}
```

## Key Loadinfo APIs Summary

| Structure | Purpose |
|-----------|---------|
| `struct _mulle_objc_loadinfo` | Main loadinfo container |
| `struct _mulle_objc_loadclass` | Class definition for loading |
| `struct _mulle_objc_loadcategory` | Category definition for loading |
| `struct _mulle_objc_loadinfo_enqueue_nofail()` | Register loadinfo with runtime |
| `MULLE_C_CONSTRUCTOR` | Automatic loading pattern |
| `mulle_objc_global_lookup_infraclass_nofail()` | Lookup loaded class |