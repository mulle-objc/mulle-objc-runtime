<!-- Keywords: test, quirks, golden, output, timeout, nondeterministic -->

# Testing quirks

- The `test/` directory is its own mulle-sde project. Top-level `mulle-sde craft`
  works on the main project, not on test state.

- If dependency state or generated test env looks stale, start with:

```bash
mulle-sde retest
```

- Do not run `mulle-test` directly when working inside a mulle-sde project. Use
  `mulle-sde test ...` so the test environment, paths, and wrappers stay correct.

- Do not run test executables directly. They may be stale and they bypass the
  managed test environment.

- Avoid wrapping `mulle-sde test run` with external `timeout`; prefer
  `mulle-sde test run --timeout ...`.

- Keep test output deterministic. Do not print pointer addresses, timestamps, or
  unordered collection dumps if the result will be compared against `.stdout`.

- Coverage runs instrument the build. Run `mulle-sde test clean tidy` before going
  back to plain `mulle-sde test run`.
