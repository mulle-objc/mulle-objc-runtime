# File Organization
<!-- Keywords: file, organization, split, add, reflect, multi-file, source, header, include -->

## Adding source files

```bash
mulle-sde add src/foo.c              # C source (creates .c + .h)
mulle-sde add -t file src/helpers.c  # plain .c file (no header template)
mulle-sde add src/bar.h              # standalone header
```

`add` creates the file from a template and runs `reflect` automatically.
After reflect, the build system knows about the new file — no manual
CMake edits needed.

## When to split

Split when a file exceeds ~300–500 lines or when you have distinct
responsibilities (e.g., data structures vs. algorithms vs. I/O).

## How includes work

After `mulle-sde reflect`, two generated headers exist:

| File | Purpose |
|------|---------|
| `src/include.h` | Include all project headers (use in .c files) |
| `src/include-private.h` | Same + private headers (use in test code) |

Every `.c` file should `#include "include.h"` at the top. Don't manually
include individual project headers — `include.h` handles the order.

## Moving / removing files

```bash
mv src/old.c src/new.c
mv src/old.h src/new.h
mulle-sde reflect           # updates cmake and include.h
```

```bash
rm src/unused.c src/unused.h
mulle-sde reflect
```

## Verify after changes

```bash
mulle-sde check             # fast compile check (~0.7s, no link)
mulle-sde craft             # full build (before run/test)
```
