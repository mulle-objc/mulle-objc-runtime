# alias-to-late-category

Negative / boundary test.

`Consumer` (a subclass of `Base`) declares a class-method alias

```objc
@method_implementation +aliasToCat = +baseValue;
```

but the alias TARGET `+baseValue` is supplied by a **category**
`Consumer( Cat)` that lives in its own translation unit and is deliberately
listed LAST in the build so it loads AFTER `Consumer`.

Unlike a `@mixin` (which the loader treats as a hard load dependency and
guarantees to be present before the adopting class is dequeued from the load
waitqueue), a category is NOT a load dependency of its class. The class loads
first; the category later. So when `Consumer`'s own method list is installed,
`+baseValue` does not exist yet and cannot be resolved.

The runtime therefore aborts loudly at load time with

```
Could not alias method "aliasToCat"
... fatal: unknown method ... "+baseValue" in class ... "Consumer"
```

That is the intended behaviour and what this test pins: aliasing a target that
only a later category provides is unsupported and must FAIL LOUDLY (a
diagnosed method-not-found abort), NOT silently corrupt the method table and
crash on dispatch (the original bug, see BUGREPORT-deferred-alias-mixin-target.md
and test class/alias-to-mixin-method.m).

If deferred/late alias resolution across categories is ever implemented (see
the bug report's "suggested fix #1", a wait-queue based retry), this test
should be converted into a positive test.
