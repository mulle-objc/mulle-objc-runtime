# LoadInfo : Loading Classes, Categories, Strings

Classes, categories, static strings are all added with the same function
`mulle_objc_loadinfo_unfailing_enqueue`. Usually the compiler produces these calls automatically for you.

## Functions

### `mulle_objc_loadinfo_unfailing_enqueue`

```
void   mulle_objc_loadinfo_unfailing_enqueue( struct _mulle_objc_loadinfo *info)
```

Take `info` and queue the contained classes, categories, strings up for inclusion into the runtime. If the base class of a class or category is already present, it will be added immediately to the runtime. Otherwise the inclusion will be delayed until the base class appears.

The **unfailing** in `mulle_objc_loadinfo_unfailing_enqueue` indicates, that the function will throw an exception or abort, if the runtime is not properly setup or if it detects errors in `info`.

This information is usually created by the compiler, but if you want to add a class at runtime, you would also do it via `mulle_objc_loadinfo_unfailing_enqueue`.

Checkout [demo1.c](../tests/demo/demo1.c) for a fairly well documented example, how to do this.

> `mulle_objc_loadinfo_unfailing_enqueue` will treat the contents of
>  `_mulle_objc_loadinfo` as if they are in read only memory, with all
> its implications, with respect to lifetime and mutability.


## Calculating the various ids

The various needed ids are simply calculated by using the `mulle_objc_uniqueid_from_string` on the name of the desired entity.  Usually the compiler will do all this for you.

Example:

```
	mulle_objc_uniqueid_t   hash;

	hash = mulle_objc_uniqueid_from_string( "Foo");
	printf( "%s : "%08lx\n", "Foo", hash);
```

There is also a standalone tool called `mulle-objc-uniqueid` that can do this for you:

```
$ mulle-objc-uniqueid Foo
40413ff3
```


## Calculating the class ivarhash

> Probably only interesting to compiler writers.

The ivarhash is the concatenation of the name and the signature, separated by ':' of each instance variable in order of `offsetof` of a class hierarchy. Each item is separated by ','.

Example:

```
@interface Foo
{
	id     a;
	double b;
}
@end
```

The string to hash is `"a" ":" @encode(id) "," "b" ":" @encode(double)`. That boils down to `"a:@,b:d"`. The FNV1-32 hash is then `0x941dbdbc`.
Now lets add a subclass:


```
@interface Bar : Foo
{
	id   c;
}
```

The inherited string to hash from Foo is `"a:@,b:d"`. Bar adds `"c" ":" @encode(id)`, which makes the complete string `a:@,b:d,c:@`. The  ivarhash  for Bar is then `0x2fb45c51`.
