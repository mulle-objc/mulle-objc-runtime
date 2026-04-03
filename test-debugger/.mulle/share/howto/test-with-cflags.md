# Testing with Custom CFLAGS
<!-- Keywords: c, test, CFLAGS -->

## Running Tests with Custom Compiler Flags

To test the project with custom CFLAGS (e.g., to enable/disable features):

```bash
mulle-sde -DCFLAGS="-DFLAG_NAME" retest
```

**Important:** Always use `retest` (not just `test`) when changing CFLAGS to ensure a clean rebuild.

## Examples

### Test with NO_MULLE__DTOA 
```bash
mulle-sde -DCFLAGS=-DNO_MULLE__DTOA retest
```

### Test with multiple flags

```bash
mulle-sde -DCFLAGS="-DDEBUG -DVERBOSE" retest
```

### Return to normal testing
```bash
mulle-sde retest
```

## Why retest?

- `test` - runs tests with existing build
- `retest` - cleans build directory and rebuilds from scratch

When CFLAGS change, you **must** use `retest` to ensure the code is recompiled with the new flags. Otherwise, the old compiled code will be used and your flags will have no effect.
