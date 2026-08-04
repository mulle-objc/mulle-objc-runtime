# Testing
<!-- Keywords: test, run, testing, workflow, valgrind, coverage, golden, retest, vibecoding -->

## Critical rule

The `test` directory is an **isolated mulle-sde project**.

If a test only passes with a workaround, **do not add the workaround** just to
make it green. Let it fail and report it as a significant finding.

In particular, do not:

- rewrite expected output (`.stdout`/`.stderr`) to hide regressions
- add bypass flags or environment overrides only for the failing case
- change test flow to skip the failing path

A failing test in the normal workflow is evidence of a product/tooling issue and
should drive a real fix.

Wrong:

```bash
mulle-sde craft
```

Correct:

```bash
mulle-sde retest # wipes stash, refetches, crafts and runs
# or
mulle-sde test craft  # keeps stash as is
mulle-sde test run
```

Always use `mulle-sde test ...` for test work. Do not call `mulle-test`
directly from project automation or agent guidance.

## Setting up testing

```bash
mulle-sde test init
mulle-sde vibecoding
```

## Running tests

```bash
mulle-sde test run                           # run all tests
mulle-sde test rerun                         # run failed and new tests
mulle-sde retest                             # complete tidy retest (expensive)
```


```bash
mulle-sde test run path/to/test                 # run a a single test
mulle-sde test run --rerun path/to/test         # only run failed tests
mulle-sde test run --timeout 10 path/to/test    # do not use unix timeout
mulle-sde test --valgrind run path/to/test      # notice position!
```

## Recommendation for graphical executables

For GUI/event-driven executables, prefer serial execution to avoid test
interference from parallel runs.

```bash
mulle-sde -d test environment --scope project set MULLE_TEST_PARALLEL NO
mulle-sde test run
```

For one-off runs, you can also force serial with:

```bash
mulle-sde test --serial run
```

Use a shorter test timeout for graphical apps:

```bash
mulle-sde -d test environment --scope project set MULLE_TEST_RUN_TIMEOUT 20
mulle-sde test run
```

For one-off runs:

```bash
mulle-sde test run --timeout 20
```

Note: `MULLE_TEST_RUN_TIMEOUT` controls `mulle-sde test run`.  
`MULLE_SDE_RUN_TIMEOUT` is for `mulle-sde run`.

## Output and golden files

```bash
cat /absolute/path/to/file.tmp.stdout
cat /absolute/path/to/file.tmp.stderr
mulle-sde test run --rerun --golden-stdout path/to/test
```

Always use absolute paths with `--golden-stdout`. Relative paths can resolve
against the wrong CWD and gold the wrong test file.

With vibecoding enabled, `*.tmp.stdout`, `*.tmp.stderr`, `*.tmp.ccerr`, and the
test `.exe` stay around for inspection.

## Test structure

- use `include.h` or `import.h`
- return `0` for success
- prefer `mulle_printf`
- avoid nondeterministic output
- compare output against `.stdout` files

For coverage-specific workflow and pitfalls, load the `coverage` file from this
bundle as well.
