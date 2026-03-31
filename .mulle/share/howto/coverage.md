# Improving Test Coverage with mulle-sde
<!-- Keywords: test, coverage, golden, ai, agent -->

### Overview

The goal is to increase line coverage by writing test files and
golding their output. Coverage is measured with `lcov` or `gcovr` via
`mulle-sde test coverage`.

---

### Step 1: Measure baseline coverage

```bash
mulle-sde test clean all
mulle-sde test coverage 2>&1 | grep -E "\.m|TOTAL"
```

This shows per-file line coverage. Focus on files with low % and many uncovered
lines.

**Important:** `mulle-sde test coverage` leaves the build in a
coverage-instrumented state. Always run `mulle-sde test clean all` before
switching back to plain `mulle-sde test run`.

---

### Step 2: Identify what to cover

Read the source file around the uncovered line numbers. The coverage report lists
them like:

```
NSArray.m   362   217   59%   91,93,107-108,...
```

Open `src/NSArray/NSArray.m` and look at those lines to understand which
methods or branches are not being exercised.

---

### Step 3: Write a test file

Place tests in `test/NN_topic/filename.m`. Use the standard preamble:

```objc
#import "import.h"

int main( void)
{
   // ... test code ...
   return( 0);
}
```

**Critical rules:**
- Use `mulle_printf` — not `printf`, not `NSLog`
- No dot-syntax: use `[obj method]` not `obj.method`
- Prefer factory methods over `alloc/init` without `-autorelease`
- Never print `allKeys`, `allValues`, or set members directly — order is
  non-deterministic. Print counts or check specific keys with `objectForKey:`
- **Platform safety**: `NSInteger`/`long` are 32-bit on Windows. Never print
  raw values of `INT64_MAX`, `UINT32_MAX` etc. through these types — use
  boolean comparisons: `value != 0 ? "yes" : "no"`
- Set `MulleObjCDebugElideAddressOutput=YES` in the test environment for
  deterministic `debugDescription` output (no pointer addresses)

---

### Step 4: Golden the test output

```bash
mulle-sde test run --rerun --golden-stdout \
    /absolute/path/to/test/NN_topic/filename.m
```

**Always use absolute paths** — relative paths resolve against the wrong CWD.

Check the generated `.stdout` file looks correct before proceeding.

---

### Step 5: Verify the test passes

```bash
mulle-sde test run --rerun \
    /absolute/path/to/test/NN_topic/filename.m
```

---

### Step 6: Run full suite and re-measure

```bash
# Verify nothing is broken
mulle-sde test run 2>&1 | tail -3

# Re-measure coverage
mulle-sde test clean all
mulle-sde test coverage 2>&1 | grep -E "\.m|TOTAL"
```

---

### Pitfall: stale LDFLAGS after coverage run

On Linux, `mulle-sde test coverage` can write a stale `gcov_stubs.o` path into
`LDFLAGS` in the env files. This breaks subsequent `test run` commands. After
every coverage run, check:

```bash
grep "LDFLAGS\|gcov_stubs" test/.mulle/etc/env/environment-global.sh
grep "LDFLAGS\|gcov_stubs" test/.mulle/etc/env/environment-user-*.sh
```

If found, remove the `LDFLAGS=...gcov_stubs...` line from those files.

### Hint for AI agents writing coverage tests

When launching an agent to write coverage tests, provide:
1. The per-file coverage table (file, lines, hit, %, uncovered line numbers)
2. The source files content around uncovered lines
3. A list of existing test files so the agent doesn't duplicate coverage
4. The absolute path prefix for `--golden-stdout`
5. Remind about non-deterministic ordering (pointers, allKeys, allValues, set members)
6. Remind about platform-safe assertions (no raw large int values)
