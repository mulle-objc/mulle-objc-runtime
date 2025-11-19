## Syntax
<!-- Keywords: syntax, styleguide, c -->

mulle-c libaries (should) use a consistent naming scheme for their data
structures and functions. After a bit of use, you should easily figure out
how a function is supposed to be named, that performs a certain action.
Conversely just by looking at the function name, you can discern the scope of
the function, its arguments and the order in which they appear.


### Data structure

A [data structure](https://en.wikipedia.org/wiki/Data_structure)
is a collection of a struct and functions operating on it.


#### Struct

If a data structure interfaces with mulle-allocator
(see: mulle-allocator) it is significant. It is distinguished by the syntax,
if the data structure keeps track of the allocator in its struct:

``` c
struct my__data
{
   void                    *bytes;
   size_t                  length;
   // no allocator, therefore my__data and not my_data
};
```

``` c
struct my_data
{
   void                    *bytes;
   size_t                  length;
   struct mulle_allocator  *allocator;
};
```


The `struct` is name-spaced with a string, which is always "mulle"
in mulle projects.

```
struct                   : struct_with_allocator
                         | struct_without_allocator
                         ;

struct_with_allocator    : namespace '_' struct_name
                         ;

struct_without_allocator : namespace '_'  '_' struct_name
                         ;

namespace                : "mulle"

struct_name              : [a-z][a-z0-9]+
                         ;
```

Notice that there are no uppercase characters and no '`_`' underscore character
in a struct_name.

A tightly coupled struct inherits the allocator identification, even if itself
has no allocator or adds an allocator. A typical case is an enumerator:

``` c
struct my_dataenumerator
{
   void     *curr;
   size_t   length;  // has no allocator, but belongs to `my_data`
                     // therefore not my__dataenumerator
};

```

##### Fields

Structs are composed of fields. It's distinguished, if a field is supposed to
be accessed via accessor functions only (private).

``` c
struct mulle__string
{
   char     *s;      // public
   size_t   _len;    // private
};

size_t   mulle__string_get_length( struct mulle__string *p);
```


```
field            : public_field
                 | private_field
                 ;

public_field     : field_name
                 ;

private_field    : '_'  field_name
                 ;

field_name       : [a-z][a-z0-9_]*
                 ;
```

Notice that no uppercase characters are part of a field_name.
Underscores following an initial letter are fine, but should not be the last
character.


#### Functions

Functions of a data structure are prefixed with the name of the struct.
What the function does, is expressed in its function_name which is most often a
verb/object combination with a predefined meaning:

``` c
size_t   mulle__string_get_length( struct mulle__string *s);
```

An unsafe_function function does not actively check the given arguments for
NULL pointers. A safe_function guarantees to not crash on NULL pointer input.


```
function            : unsafe_function
                    | safe_function
                    ;

unsafe_function     :  '_' struct '_' function_name
                    ;

safe_function       :  struct '_' function_name
                    ;

scoped_function_name :  scope '_' function_name
                     | function_name
                     ;

scope               : [a-z][a-z0-9]+
                    | "global"
                    | "universe"
                    | "thread"
                    ;

function_name       : verb '_' object modifiers
                    | verb modifiers
                    ;

verb                : [a-z][a-z0-9]+
                    | "advance"
                    | "alloc"
                    | "copy"
                    | "count"
                    | "create"
                    | "destroy"
                    | "done"
                    | "enumerate"
                    | "extract"
                    | "find"
                    | "free"
                    | "get"
                    | "guarantee"
                    | "init"
                    | "insert"
                    | "lookup"
                    | "make"
                    | "member"
                    | "push"
                    | "pop"
                    | "next"
                    | "register"
                    | "remove"
                    | "reset"
                    | "reserve"
                    | "search"
                    | "set"
                    ;

object              : [a-z][a-z0-9_]*
                    | "allocator"
                    | "bytes"
                    | "count"
                    | "data"
                    | "items"
                    | "last"
                    | "length"
                    | "notakey"
                    | "string"
                    | "size"
                    | "size_as_length"
                    | "used"
                    | "used_as_length"
                    ;

modifiers           :
                    : '_' modifier modifiers
                    ;

modifier            : [a-z][a-z0-9]+
                    | inline
                    | nofail
                    ;
```

## Semantics

### Verbs

Generally verbs act on the first parameter of the function call.
(e.g. mulle_map_get( map)). If that is not the case the verb is
preceded by the scope of the verb. That could be "thread_get" for
example for accessing thread-local storage.

#### Verb considerations

* If there is a function where there are source and destination arguments, the
destination argument comes first.
* If a pointer along with a length are being passed, it is preferred to use
something like `struct mulle_data` and pass the couple by value (sic!). Otherwise pass
the pointer first, immediately followed by the length.
* a by-reference return value is always the last argument (multiple reference values can be
avoided with little structs)
* an allocator is the last argument, unless a by-reference return value is present


#### Initialization and Destruction

| Verb    | Meaning
|---------|---------------
| alloc   | allocate memory from heap
| copy    | create + copy items
| done    | tear down struct on stack or heap
| free    | deallocate memory to heap
| init    | setup struct on stack or heap
| make    | construct on the fly (e.g. return a struct by value)

| Verb    | Decomposition
|---------|---------------
| create  | alloc + init
| destroy | done + free
| reset   | remove all, keep allocation though

| Verb    | Opposite
|---------|----------
| alloc   | free
| copy    | destroy
| create  | destroy
| init    | done


#### Memory management

| Verb      | Opposite
|-----------|----------
| guarantee | Ensure that there is space for 'n' elements without realloc. A pointer to the guaranteed memory area is returned
| advance   | Reserve space for 'n' elements, returning a pointer to the first space
| reserve   | Reserve space for 1 element, returning a pointer to it

#### Element Access Read

| Verb        | Meaning
|-------------|---------------
| get         | random access read
| next        | sequential access read
| member      | returns 1 (yes) or 0 (no) depending on presence in container


#### Element Access Write

| Verb        | Meaning
|-------------|---------------
| add         | sequential access write, into empty space
| extract     | destructive get, with ownership transfer (caller must free)
| insert      | random access write, not overwriting (may return success)
| member      | returns 1 (yes) or 0 (no) depending on presence in container
| push        | add to end
| pop         | get and remove from end
| register    | a get or set operation, returns the previous value , with ownership | transfer (caller must free)
| remove      | random access remove, always returns `void` (sic!)
| set         | a destructive insert (does not return previous value)
| update      | a destructive insert (returns the previous value, if ownership transfer permits it)

A register does not destroy a previously inserted value, an exchange does.


#### Element Searches

Cache verbs are slightly different (see below)

| Verb        | Meaning
|-------------|---------------
| enumerate   | create enumerator
| find        | a search that is linear, returns an index (mulle_not_found_e) or key
| lookup      | a hashtable or similiar indexing (but not a get)
| search      | a search that's not linear, probably a binary search, returns the value
| count       | get number of elements (non char or byte), probably a "find"


| Verb/Object | Meaning
|-------------|---------------
| copy_items  | copy each item from source to destination struct [ dst, src]


#### Misc. Operations on Caches and Containers with Caches

A cache is a non-complete subset of a larger backing store. The backing store
can be part of the data-structure or it can be separate.

| Verb        | Meaning
|-------------|-----------
| fill        | just a cache update with no previous probe or search
| find        | a search that is linear, returns an index (mulle_not_found_e) or key
| lookup      | a cache lookup probe followed possibly by a search and refresh if the lookup failed.
| probe       | a hashtable or similiar indexing (but not a get)
| refresh     | a search with a cache update (must not probe)
| search      | a search that's not linear, probably a binary search, returns the value


### Objects

The verb may be followed by an object, like for example in `get_length`, get
is the **verb** and **length** is the object. That this is called an "object"
is purely linguistically, you can also call it "attribute" or "field" or
"property".
There are a few standardized objects with specific meanings. Underscores
following an initial letter are fine, but should not be the last character.

| Object         | Meaning
|----------------|----------
| capacity       | the desired user capacity, not the actual size, which will be larger when allocated
| count          | a quantity of something other than bytes (usually pointers)
| length         | a quantity in bytes or characters
| notakey        | the value used for indicating an invalid key (often NULL)
| size           | the current maximum quantity (max count)
| size_as_length | as above but in bytes
| used           | the amount of the maximum quantity in use (like count, but there could be holes that make used larger than count)
| used_as_length | as above but in bytes

If there are multiple verb modifiers, append them in sorted order.


### Modifiers

After the object, or if no object is present after the verb, there can be a
number of modifiers. These are concatenated in sorted order.

| Verb        | Meaning
|-------------|----------
| inline      | A inlining variant of another non-inlining function
| nofail      | The function will never fail, like a search that is guaranteed
|             | to find something. The function may throw, abort or exit
