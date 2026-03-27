# Testing
<!-- Keywords: test, run, testing, workflow, valgrind, coverage -->


## 🚨 CRITICAL RULE - READ THIS FIRST 🚨

**`test` is an ISOLATED mulle-sde project**

The test directory is  as self-contained mulle-sde project with its own
`dependency-dir` and `kitchen-dir`. The main project is only a dependency of
the test project.

### ❌ WRONG - This affect the MAIN project, NOT the test:

```bash
mulle-sde craft
```

### ✅ CORRECT - These affect the TEST project:

```bash
mulle-sde retest
mulle-sde test craft
mulle-sde test run
```

**Always use `mulle-sde test` to deal with the test environment, never use
`mulle-test` directly.**


## Setting up Testing

### Initialize test directory

**NEVER EVER** use just `mkdir` instead do it properly:

```bash
mulle-sde test init
```

### Enable vibecoding

```bash
mulle-sde vibecoding
```

Enable vibecoding to get persistent stdout and stderr logs as `*.test.*` files
and make your life much easier. With vibecoding enabled, the files generated
by the test will be besides the testfile. Also the generated test executable
with the suffix `.exe` will not be removed.

## Test Result Files

- `.test.stdout` - actual test output
- `.test.stderr` - runtime errors
- `.test.ccerr` (or `.test.ccdiag`) - compiler warnings/errors
- `.exe` - compiled executable (don't run directly)


## Running Tests

### Run a specific test

```bash
mulle-sde test run path/to/filename[.extension]
```

Exit status 0 = PASS!

### Run with timeout

Careful! A mulle-sde test run has already a default timeout in vibeocoding
mode. DO NOT RUN mulle-sde test run within another `timeout`, instead modify
the timeout:

```
mulle-sde test run --timeout 10 path/to/filename[.extension]
```

### Run with sanitizers

```bash
mulle-sde test [sanitizers] run <path_to_test_file>
```

*Example: `mulle-sde test --valgrind run test/NSTableView/01_initialization.m`*


### Run all tests

```bash
mulle-sde test run
```

### Retest everything

If you are working on more than one project in parallel.

**IT IS CRUCIAL TO `RETEST` TO GET DEPENDENCY CHANGES**

```bash
mulle-sde retest
```

This will fetch dependencies anew and wipe the old kitchen and dependency
directories.

### Build and test cycle

With vibecoding enabled, just run the test, mulle-sde test will rebuild:

```bash
mulle-sde test run path/to/filename.extension
```

## Checking Test Output

Compiler errors go to `*.test.ccerr`
Check `.test.stdout|stderr` files for actual output.

```bash
# Output is in .test.stdout file and stderr
cat path/to/filename.test.stdout
cat path/to/filename.test.stderr
```


```bash
mulle-sde test run --golden-stdout <testname>
```

## Creating a Test

Create a theme directory below test like `20-leak`

Create `20-leak/mytest.c` in your project. Write your test code (just a normal
C program with main()). You can write result values to stdout.

### Test Structure Example

In well behaved mulle project you can just "include.h" or "import.h" in your
test and you will get all dependency headers.

```c
#include "include.h"  // use provided header for all dependency includes

int   main( void)
{
   // 1. Setup objects and environment
   // 2. Print descriptive output of the test steps and results
   mulle_printf("Test description: ...\n");
   // 3. Clean up and deallocate all objects
   return 0;  // 0 = success, non-zero = failure
}
```

- Tests are compiled with `-DMULLE_TEST=1`
- Return 0 for success, anything else for failure
- Prefer mulle_printf over printf, if available
- **NEVER print**: times, dates, pointer addresses, error codes, anything that will vary between two test runs
- **Output-Based Verification:** Tests verify correctness by comparing `mulle_printf` output to the corresponding `.stdout` file. Avoid using traditional assertion libraries
- **Descriptive Output:** The `mulle_printf` output should be clear and descriptive, making it easy to understand what is being tested


## Testing Guidelines

- Each feature or major function should have its own test file
- Test files should be organized in the `test/` directory, with subdirectories reflecting the component under test
- Test files can be accompanied by a `.stdout` file containing the exact expected output
- Rigorously test edge cases, especially `nil` handling for all relevant parameters
- Ensure all allocated objects are properly deallocated within the test to prevent leaks
- Each test should be small, focused, and test one specific piece of functionality

## Creating Expected Output

Once you are really sure that the test output is correct, you can conveniently
create the .stdout file with:

## DON'T DO THIS - Wrong Approaches

### ❌ Never run mulle-test directly
**Problem**: Missing environment variables, wrong paths, confusing errors you can't figure out

### ❌ Do not run executables directly
**Problem**: There is a huge danger of running stale tests, which give old results.

