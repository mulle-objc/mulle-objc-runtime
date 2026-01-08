# mulle-sde Debug Crashes Guidelines
<!-- Keywords: debug, crash, stacktrace  -->


## Workflow

**When code crashes: STACKTRACE FIRST, fix SECOND.**

### 1. Get stacktrace

```bash
mulle-sde debug stacktrace [executable] -- [arguments]
```

### 2. Read it

```
#0  0x5555555a121d in nvg__allocPathCache () at nanovg.c:246
#1  0x5555555a1985 in nvgCreateInternal (params=...) at nanovg.c:440
#2  0x555555569c04 in main (argc=1, argv=...) at main.c:259
```

Frame #0 = crash location. Look at that line in that file.

### 3. Fix the actual problem

Don't guess. Don't add random checks. Try to fix what the stacktrace shows
first.

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

For other debugging options check out

``` bash
mulle-sde debug help
```
