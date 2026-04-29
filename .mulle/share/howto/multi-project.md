# Working with multiple mulle projects
<!-- Keywords: mulle-sde, dependency, multi-project, multiple, projects, cache -->

## Key Concepts

Each project has these three "ephemeral" directories containing:

- `stash/` — fetched dependency sources
- `kitchen/` — build artifacts (cmake build dirs, logs)
- `dependency/` — installed headers + libs (read-only after install)

The location of these directories can be changed: `mulle-sde -s env | egrep 'DEPENDENCY_DIR|KITCHEN_DIR|MULLE_SOURCETREE_STASH_DIR'` but often reside in:
`~/.mulle/var/cache/sde/${PROJECT_NAME}-${MULLE_VIRTUAL_ROOT_ID}/`

Contents of `stash/` are **fetched** (`mulle-sde fetch`)
Contents of `kitchen/` are **crafted** (cmake build output)
Contents of `dependency/` are **installed** (headers + libs from kitchen)

### The test project is a separate project

`mulle-sde test init` creates a `test/` subdirectory that is its **own mulle-sde project**.
It has its own sourcetree, stash, kitchen, and dependency cache — separate from the main project.
The library under test is just another dependency of the test project (symlinked to `..` when local).

This means:
- `mulle-sde retest` runs **inside the test project** — it rebuilds the library + runs tests
- The test project's kitchen logs are at a **different cache path** than the main project's
- Changes to the main project source are picked up automatically by `retest` (via symlink)
- `mulle-sde craft` in the main project does NOT run tests

---

## Daily Workflow

### Build a project
```bash
cd ProjectA
mulle-sde craft              # incremental build
mulle-sde craft --clean      # clean project build then rebuild (keeps dependency/ cache)
mulle-sde -f craft           # force full rebuild including re-fetch/reflect/craftorder
```

> **In vibecoding:** Use `mulle-sde -f craft` ONLY for reflect/reamalgamate. For normal
> development, use `mulle-sde test craft` (see below).

### Run tests (vibecoding: use this for development)
```bash
mulle-sde test craft         # rebuild test binary + parent lib (NOT deps) — FAST
mulle-sde test run           # run tests without rebuilding
mulle-sde retest             # rebuild lib + ALL deps + run tests — SLOW, use sparingly
```

> **In vibecoding**: `mulle-sde test craft` is the main development loop. It rebuilds the
> library under test and the test binary, but NOT the dependencies. Much faster than `retest`.
> Only use `retest` when you've changed a dependency or need a full clean rebuild.

### After adding/removing/moving source files
```bash
mulle-sde reflect            # regenerates cmake/_Sources.cmake, _Headers.cmake etc.
```

### After adding/removing a dependency
```bash
mulle-sde dependency add <url>   # add
mulle-sde dependency remove <name>
mulle-sde reflect                # regenerate cmake files
```

---

## Cleaning (escalating order)

```bash
mulle-sde clean              # clean project build only (keeps deps)
mulle-sde clean all          # clean project + dependency/ folder → full dep rebuild next time
mulle-sde clean tidy         # clean everything except archive/graveyard (slow, nukes stash)
mulle-sde -f clean mirror    # clean repo mirror cache
```

> **Rule of thumb** (from mulle-sde docs):
> - Something wrong → `clean all`
> - Still wrong → `clean tidy` (then `mulle-sde fetch` to re-fetch stash)

After `clean tidy`, re-fetch stashed dependencies (or just craft again):

```bash
mulle-sde fetch
```


## Working on ProjectA while building ProjectB

ProjectB's `mulle-sde retest` rebuilds ProjectA from source automatically (via symlink in stash).
You should ensure that ProjectA has up to date objc-deps.inc by running a
`mulle-sde -f craft` there first.

```bash
cd ProjectA && mulle-sde -f craft   # updates objc-deps.inc + installs headers
cd ProjectB && mulle-sde retest     # picks up ProjectA changes automatically
```

> Note: plain `mulle-sde craft` in ProjectB does NOT rebuild ProjectA — it only rebuilds
> ProjectB itself. Use `retest` or `-f craft` to trigger dependency rebuilds.

### The "Stale Header" Trap

**Symptom:** You change ProjectA's header, run `mulle-sde test craft` in ProjectB, but the old
header is still used. Looks like a cmake cache bug.

**Reality:** `test craft` rebuilds ProjectB (the parent lib) but NOT its dependencies. ProjectB's
test project's `dependency/` cache still has the old ProjectA headers. This is expected — it's
what makes `test craft` fast.

**Fix:** Use `retest` when you've changed a dependency:
```bash
cd ProjectB && mulle-sde retest      # rebuilds ProjectA + ProjectB + runs tests (SLOW)
```

Or if you only changed ProjectA and want to update ProjectB's test cache:
```bash
cd ProjectA && mulle-sde -f craft    # updates objc-deps.inc + installs to its own cache
cd ProjectB && mulle-sde retest      # picks up ProjectA changes
```

If you're stuck with stale headers and `retest` doesn't help:

```bash
mulle-sde -f clean tidy && mulle-sde retest   # wipe dependency/ cache and rebuild
```

---

Assume ProjectB is depending on an amalgamation of many
other projects called ProjectX, which contains ProjectA, but not directly on
ProjectA. If you changed ProjectA you still only need to do:

```bash
cd ProjectA && mulle-sde -f craft
cd ProjectB && mulle-sde retest
```

The `mulle-sde -f craft` will also reamalgamate ProjectX
(see hook `mulle-sde env get MULLE_CRAFT_POST_PROJECT`)!
So this will work. What will NOT work is is editing ProjectX, your codes will
be lost on the next reamalgamation.

---

## Checking What's Wrong

Avoid running multiple craft or test commands to grep for strings. This takes
too long. Instead redirect into a log file and grep that.

### Find compile errors
```bash
# Errors only, filter noise
mulle-sde test craft > log 2>&1 ; grep "error:" log | grep -v "mulle-craft\|mulle-make\|Parallel\|ld.lld\|undefined symbol"
```

### Find linker errors
```bash
mulle-sde retest >log 2>&1 ; grep "undefined reference\|undefined symbol" log | sort -u
```

### Read build logs directly
```bash
# Logs are in the kitchen
ls ~/.mulle/var/cache/sde/<ProjectName>-<hash>/kitchen/.craftorder/Debug/<ProjectName>/.log/
# 00.cmake.log = configure, 01 = build, 02 = install, 03 = cmake generate, 04 = compile
tail -20 ~/.mulle/var/cache/sde/<ProjectName>-<hash>/kitchen/.craftorder/Debug/<ProjectName>/.log/04.cmake.log
```

---

## Dependency Management

Read the `mulle-sde dependency howto`!


---

## Auto-generated Files — DO NOT EDIT

These are regenerated by `mulle-sde reflect` or `mulle-sde -f craft`:

``` bash
cmake/reflect/_Sources.cmake     # source file list
cmake/reflect/_Headers.cmake     # header file list + install dirs
src/reflect/objc-deps.inc        # ObjC class/category dependency list
src/reflect/_<Project>-export.h  # public header aggregator
src/reflect/_<Project>-include.h # private include aggregator
```

If you need to add include dirs or install extra headers, edit:

``` bash
cmake/Headers.cmake              # custom header install rules (safe to edit)
```

---

## Common Mistakes

| Mistake | Fix |
|---------|-----|
| Editing `objc-deps.inc` manually | Don't — run `mulle-sde -f craft --clean` to regenerate |
| Editing files in `cmake/share/` | Don't — they're shared infrastructure |
| Running `retest` repeatedly hoping it fixes itself | Diagnose first with log files |
| Hacking env vars or build system files directly | Use `mulle-sde environment set` / `mulle-sde definition set` |
