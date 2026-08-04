# Improving Test Coverage with mulle-sde
<!-- Keywords: test, coverage, golden, ai, agent -->

## Measure baseline coverage

```bash
mulle-sde test clean all
mulle-sde test coverage 2>&1 | grep -E "\\.m|TOTAL"
```

After a coverage run, clean again before going back to plain test runs.

## Write focused tests

Place tests in `test/NN_topic/file.m`, use `mulle_printf`, and keep output
deterministic.

## Golden the output

```bash
mulle-sde test run --rerun --golden-stdout /absolute/path/to/test/file.m
```

Always use absolute paths.

## Verify and re-measure

```bash
mulle-sde test run --rerun /absolute/path/to/test/file.m
mulle-sde test run 2>&1 | tail -3
mulle-sde test clean all
mulle-sde test coverage 2>&1 | grep -E "\\.m|TOTAL"
```
