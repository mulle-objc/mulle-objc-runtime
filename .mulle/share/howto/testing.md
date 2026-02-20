# Testing
<!-- Keywords: test, run, testing, workflow, valgrind, coverage -->



## 🚨 CRITICAL RULE - READ THIS FIRST 🚨

**`test` is an ISOLATED mulle-sde project**

The test directory is  as self-contained mulle-sde project with its own
`dependency-dir` and `kitchen-dir`. The main project is only a dependency of
the test project.

### ❌ WRONG - These affect the MAIN project, NOT the test:

```bash
mulle-sde craft
mulle-sde clean
mulle-sde fetch
mulle-test run
```

### ✅ CORRECT - These affect the TEST project:

```bash
mulle-sde retest
mulle-sde test craft
mulle-sde test run
```

**Always `cd` into the `test` directory before executing commands**

This is super important. It ensures, that you don't run commands "accidentally"
in the main project directory and get confused by results not showing up in the
test project.

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

## Creating a Test

Create a theme directory below test like `20-leak`

Create `20-leak/mytest.c` in your project. Write your test code (just a normal
C program with main()). You can write result values to stdout.

### Test Structure Example

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

In well behaved mulle project you can just "include.h" or "import.h" in your
test and you will get all dependency headers. If this is not working look at
the contents of "include.h" or "import.h", and pick the headers you need.

## Running Tests

### Run a specific test

```bash
cd test && mulle-sde test run path/to/filename[.extension]
```

Exit status 0 = PASS!

### Run with timeout

Careful! A mulle-sde test run has already a default timeout in vibeocoding
mode. DO NOT RUN mulle-sde test run within another `timeout`, that will not
work well. Instead modify the timeout like this:

```
mulle-sde test run --timeout 10 path/to/filename[.extension]
```

### Run with sanitizers

```bash
cd test && mulle-sde test [sanitizers] run <path_to_test_file>
```

*Example: `mulle-sde test --valgrind run test/NSTableView/01_initialization.m`*

Sanitizers are platform dependent. Check `mulle-test sanitizers` for a list
of supported sanitizers.

### Run all tests

```bash
cd test && mulle-sde test run
```

### Retest everything

**IT IS CRUCIAL TO TRY `RETEST` AT LEAST ONCE WHEN PROBLEMS ARISE**

```bash
cd test && mulle-sde retest
```

This will fetch dependencies anew and wipe the old kitchen and dependency
directories.

### Build and test cycle

With vibecoding enabled, just run the test, mulle-sde test will rebuild:

```bash
cd test && mulle-sde test run path/to/filename.extension
```

## Checking Test Output

Compiler errors go to `*.test.ccerr`
Check `.test.stdout|stderr` files for actual output.

```bash
# Output is in .test.stdout file and stderr
cat path/to/filename.test.stdout
cat path/to/filename.test.stderr
```

## Creating Expected Output

Once you are really sure that the test output is correct, you can conveniently
create the .stdout file with:

```bash
cd test && mulle-sde test run --golden-stdout <testname>
```

## Testing Guidelines

### Test Organization

- Each feature or major function should have its own test file
- Test files should be organized in the `test/` directory, with subdirectories reflecting the component under test
- Test files must be accompanied by a `.stdout` file containing the exact expected output
- Run the tests from the project directory. Do not `cd` to any test directory
- After a failed test, there will be an executable with a '.exe' extension besides the test source. You can run this to observe stdout and stderr better or to run the executable in a debugger

### Test Style & Philosophy

- **Output-Based Verification:** Tests verify correctness by comparing `mulle_printf` output to the corresponding `.stdout` file. Avoid using traditional assertion libraries
- **Focus and Minimalism:** Each test should be small, focused, and test one specific piece of functionality
- **Edge Cases:** Rigorously test edge cases, especially `nil` handling for all relevant parameters
- **Memory Management:** Ensure all allocated objects are properly deallocated within the test to prevent leaks
- **Descriptive Output:** The `mulle_printf` output should be clear and descriptive, making it easy to understand what is being tested
- **Mutation Safety:** For any tests involving collection enumeration, include scenarios where the collection is modified during enumeration to ensure robustness
- **NO Pointer or Date output**: The mulle_printf output must be deterministic, therefore should not contain memory addresses, dates, random values, pids etc. that are different on each run or for other users

### Testing Tips and Common Pitfalls

If you have problems with missing symbols, you need to get on top of **dependencies**.
Read the `mulle-sde howto dependencies`.

A NULL struct mulle_allocator * is almost always fine

## DON'T DO THIS - Wrong Approaches

### ❌ Never run mulle-test directly
**Problem**: Missing environment variables, wrong paths, confusing errors you can't figure out

### ❌ Do not run executables directly
**Problem**: There is a huge danger of running stale tests, which give old results. This invariably leads to bad mental state and subsequent catastrophic decisions.

## Advanced Testing Topics

For specialized testing scenarios, see these howtos:

- **Coverage testing**: `mulle-sde howto show coverage`
- **Debugging crashes**: `mulle-sde howto show gdb-stacktrace`
- **Custom compiler flags**: `mulle-sde howto show test-with-cflags`
- **CMake integration**: `mulle-sde howto show test-with-cmake`
