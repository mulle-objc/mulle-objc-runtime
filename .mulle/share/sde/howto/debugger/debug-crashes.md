# mulle-sde Debug Crashes Guidelines
<!-- Keywords: debug, debugging, crash, stacktrace -->

## Workflow

**When code crashes: stacktrace first, fix second.**

### 1. Get a stacktrace

```bash
mulle-sde debug stacktrace [executable] -- [arguments]
```

### 2. Read it

Frame `#0` is the crash location. Look at that line in that file first.

### 3. Fix the actual problem

Do not guess. Try to fix what the stacktrace actually shows.

## Interactive debugging

```bash
gdb ./executable
(gdb) run
(gdb) bt
(gdb) frame 2
(gdb) print variable
(gdb) info locals
```

## More

```bash
mulle-sde debug help
```
