# Signature

The type of a method, property, or a variable is represented in the runtime
by its signature. A signature looks like this for a method returning an integer for two **id** parameters: `i@:@@`. A signature is an ASCII string.

The easiest way to get the signature for a C-type is to use
`@encode()`. See [Type Encodings](//developer.apple.com/library/content/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html)

## Types

The `@encode` types are basically compatible to Mac OS X encode.
**mulle-objc** has three additions to make signatures more expressive:


Macro          | Value  | Description
---------------+--------+-----------------
`_C_ASSIGN_ID` |  '='   | An object with "assign" semantics (-> delegate)
`_C_COPY_ID`   |  '~''  | An object with "-copy" semantics
`_C_ID`        | '@'    | Still an object as it always has been. It is synonym to `_C_RETAIN_ID`
`_C_RETAIN_ID` |  `_C_ID` | An object with "-retain" semantics


The runtime has no problems with the `long double` type. So `@encode( long double)`
should return 'D'.

There are a few encode characters which are not supported or meaningless now.
These might be reused in the future.


Macro            | Value  | Description
-----------------+--------+-----------------
`_C_BYCOPY`      | 'O'    | bycopy
`_C_BYREF`       | 'R'    | byref
`_C_CONST`       | 'r'    | const
`_C_GCINVISIBLE` | '!'    | ???
`_C_IN`          | 'n'    | in
`_C_INOUT`       | 'N'    | inout
`_C_ONEWAY`      | 'V'    | oneway
`_C_OUT`         | 'o'    | out
`_C_ATOM`        | '%'    | ???


## Functions

### `mulle_objc_signature_supply_next_typeinfo`

```
char    *mulle_objc_signature_supply_next_typeinfo( char *types, struct mulle_objc_typeinfo *info)
```

Parse the information contained in the signature string `types` into `info`.
Returns the beginning of the next unparsed character, which is useful for
traversing all types of the method, which are in order:

* return value
* self
* _cmd
* parameters

The meta-ABI is not relevant when parsing signatures.



### `mulle_objc_signature_supply_next_typeinfo`

```
char    *mulle_objc_signature_next_type( char *types, struct mulle_objc_typeinfo *info)
```

Useful for skipping a type signature.


### `mulle_objc_signature_supply_size_and_alignment`

```
char   *mulle_objc_signature_supply_size_and_alignment( char *type, unsigned int *size, unsigned int *alignment);
```

Get the `size` and `alignment` of `type`. Returns the beginning of the next
unparsed character.


### `mulle_objc_signature_supply_size_and_alignment`

```
unsigned int    mulle_objc_signature_count_typeinfos( char *types);
```

Get the number of different types contained in `types`.


### `mulle_objc_signature_get_metaabiparamtype`

```
int   mulle_objc_signature_get_metaabiparamtype( char *types);
```

Examines a method signature `types` and returns the proper meta-ABI call
convention to use:

Return Value | Description
-------------|-----------------------
-1           | error, check `errno`
0            | void method, no parameter
1            | cast argument to `void *`, cast return value from `void *`
2            | use a meta-ABI parameter block


### `mulle_objc_signature_get_metaabireturntype`

```
int   mulle_objc_signature_get_metaabireturntype( char *types);
```

This function only looks at the return value type and produces the same return value as `mulle_objc_signature_get_metaabiparamtype`.
