# mulle-sde Dependency Addition Examples
<!-- Keywords: mulle-sde, sourcetree, add, copy, dependency, library -->


## If a dependency is not a library

``` bash
mulle-sde dependency mark <name> no-link
mulle-sde dependency mark <name> no-header
```

## Copy dependencies from another project

You are in your project and want to copy "Foo" from `/tmp/other-project`

``` bash
mulle-sourcetree rcopy /tmp/other-project Foo
```

You can clobber your projects with all the dependencies and libraries of
another project like this:

``` bash
mkdir -p .mulle/etc/sourcetree
cp /tmp/other-project/.mulle/etc/sourcetree/config .mulle/etc/sourcetree/
mulle-sourcetree reuuid    # important!!
```

## Reordering dependencies

The order of the dependencies is very important. Base dependencies need to come
first. You can reorder with the `mulle-sde move` command.


``` bash
mulle-sde move MulleObjC below MyLibrary
mulle-sde move mulle-testallocator to bottom
```

## Adding the right startup library

For executables you need to match the startup library with the library you are
building on top of.

You do a `mulle-sourcetree list -r` and then consult this table from top to
bottom.

| You have                      | Startup Library
|-------------------------------|--------------------------
| `Foundation`                  | `MulleFoundation/Foundation-startup`
| `MulleFoundation`             | `MulleFoundation/MulleFoundation-startup`
| `MulleFoundationBase`         | `MulleFoundation/MulleObjCStandardFoundation-startup`  (sic)
| `MulleObjCStandardFoundation` | `MulleFoundation/MulleObjCStandardFoundation-startup`
| `MulleObjC`                   | `mulle-objc/MulleObjC-startup`
| `mulle-objc-runtime`          | `mulle-objc/mulle-objc-runtime-startup`

Example: You don't see neither`Foundation` nor `MulleFoundation` but you see
all the others you pick `MulleObjCStandardFoundation-startup`.

You add the library with
`mulle-sde add --startup github:MulleFoundation/MulleObjCStandardFoundation-startup`.
Make it the bottom dependency (or second to bottom if mulle-testallocator is
present).

## Error Messages and Solutions

### `mulle-sde fatal error: Did not find a linkable 'mulle-scion' library ...`

Look in the depedency folder `mulle-sde dependency-tree` if this is not really
a library, mark it as such: `mulle-sde mark mulle-scion no-link`.

If its build under a different name, alias it: `mulle-sde set mulle-scion aliases 'alias1`


