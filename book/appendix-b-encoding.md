# Appendix B: Objective-C Type Encoding Characters

This appendix documents the Objective-C type encoding characters supported by mulle-objc-runtime. These characters are used in method signatures to describe parameter and return types.

## Basic Type Encodings

| Encoding | C Type | Description |
|----------|--------|-------------|
| `c` | `char` | Signed 8-bit integer |
| `i` | `int` | Signed 32-bit integer |
| `s` | `short` | Signed 16-bit integer |
| `l` | `long` | Signed 32/64-bit integer (platform dependent) |
| `q` | `long long` | Signed 64-bit integer |
| `C` | `unsigned char` | Unsigned 8-bit integer |
| `I` | `unsigned int` | Unsigned 32-bit integer |
| `S` | `unsigned short` | Unsigned 16-bit integer |
| `L` | `unsigned long` | Unsigned 32/64-bit integer (platform dependent) |
| `Q` | `unsigned long long` | Unsigned 64-bit integer |
| `f` | `float` | 32-bit floating point |
| `d` | `double` | 64-bit floating point |
| `D` | `long double` | Extended precision floating point |
| `B` | `BOOL` | Boolean type (typedef for signed char) |
| `v` | `void` | Void type |

## Object and Pointer Types

| Encoding | C Type | Description |
|----------|--------|-------------|
| `@` | `id` | Object reference (retained) |
| `~` | `id` | Object reference (assigned) |
| `#` | `Class` | Class object |
| `:` | `SEL` | Selector |
| `*` | `char *` | C string pointer |
| `^` | `void *` | Generic pointer |
| `?` | `void *` | Undefined type (used for function pointers) |

## Structure and Union Types

| Encoding | C Type | Description |
|----------|--------|-------------|
| `{`name`=`...`}` | `struct` | Structure type |
| `(`name`=`...`)` | `union` | Union type |
| `[`size`]` | Array | Fixed-size array |
| `b`number | Bitfield | Bitfield type |

## Vector and Special Types

| Encoding | C Type | Description |
|----------|--------|-------------|
| `!` | `vector` | Vector type |
| `%` | `atom` | Atom type (used by some compilers) |

## Function Pointer Encodings

Function pointers use a nested encoding format:
- `?` indicates a function pointer
- The return type appears first
- Argument types follow in parentheses

Example: `i@:f` means a function returning `int` with parameters `id` and `SEL` and `float`

**Note**: mulle-objc-runtime does not support Objective-C blocks (`^?` encoding).

## Type Qualifiers (Unsupported)

The following type qualifiers are defined in the source but **not supported** by mulle-objc-runtime 0.17 and later:

| Encoding | Qualifier | Status |
|----------|-----------|--------|
| `r` | `const` | **Not supported** |
| `n` | `in` | **Not supported** |
| `N` | `inout` | **Not supported** |
| `o` | `out` | **Not supported** |
| `O` | `bycopy` | **Not supported** |
| `R` | `byref` | **Not supported** |
| `V` | `oneway` | **Not supported** |
| `\|` | `gcinvisible` | **Not supported** |

## How to Inspect Type Encodings

You can inspect type encodings using the compiler's `@encode()` directive:

```c
#include <stdio.h>
#include <objc/objc.h>

#define show_encode(x) printf("%s=%s\n", #x, @encode(x))

int main(void)
{
    show_encode(int);
    show_encode(float);
    show_encode(id);
    show_encode(SEL);
    show_encode(char *);
    show_encode(struct { int x; float y; });
    return 0;
}
```

## Signature Parsing

The runtime provides functions to parse and validate type signatures:

- `_mulle_objc_signature_supply_typeinfo()` - Parse type information
- `mulle_objc_signature_next_type()` - Skip to next type in signature
- `mulle_objc_signature_contains_object()` - Check if type contains object references
- `mulle_objc_signature_count_typeinfos()` - Count types in signature

## Memory Layout Considerations

Type encodings affect:
1. **Method signature matching** - Signatures must match exactly
2. **Argument passing conventions** - Determines register vs stack usage
3. **Structure layout** - Size and alignment requirements
4. **Memory management** - Object types require retain/release handling

## Platform Differences

- `long` and `unsigned long` (`l`/`L`) vary in size between 32-bit and 64-bit platforms
- Vector types (`!`) may have platform-specific implementations
- Structure alignment can vary by architecture

## Validation Rules

Valid signatures must:
- Use only supported encoding characters
- Have balanced brackets for structures/unions
- Properly terminate with null character
- Match the actual C types used in method implementations

## Common Patterns

### Instance Method Signature
```
@16@0:8
```
- Return: `@` (id)
- Self: `@0` (id at offset 0)
- Selector: `:8` (SEL at offset 8)

### Class Method Signature  
```
@16@0:8
```
- Same as instance method, but called on metaclass

### Method with Parameters
```
@24@0:8@16
```
- Return: `@` (id)
- Self: `@0` (id at offset 0)
- Selector: `:8` (SEL at offset 8)
- Parameter: `@16` (id at offset 16)