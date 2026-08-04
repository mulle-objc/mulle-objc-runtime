# mulle-sde Dependency Addition Examples
<!-- Keywords: mulle-sde, sourcetree, add, copy, dependency, library -->

## Mark a non-linkable dependency

```bash
mulle-sde dependency mark <name> no-link
mulle-sde dependency mark <name> no-header
```

## Copy dependencies from another project

```bash
mulle-sourcetree rcopy /tmp/other-project Foo
```

## Reorder dependencies

```bash
mulle-sde move mulle-allocator below mulle-buffer
mulle-sde move mulle-testallocator to bottom
```

## Add the right startup library

Consult the dependency list and add the matching startup library for the stack
you build on top of.
