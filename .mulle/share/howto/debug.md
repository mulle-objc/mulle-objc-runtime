# mulle-sde Debug Crashes Guidelines
<!-- Keywords: debug, debugging, crash, stacktrace  -->


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

## Compile-time Debug/Trace Flags

Many libraries provide compile-time debugging flags (e.g.,
`CAAnimationDebuggingFlags`). Enable them with `-DCFLAGS`:

```bash
mulle-sde -DCFLAGS="-DCAAnimationDebuggingFlags=0x5" craft --clean
mulle-sde run
```

Use `--clean` because cached object files won't pick up new defines.

To pass multiple flags:

```bash
mulle-sde -DCFLAGS="-DFLAG_A=1 -DFLAG_B=2" craft --clean
```

To return to normal:

```bash
mulle-sde craft --clean
```

## More

For other debugging options check out

``` bash
mulle-sde debug help
```

## Winedbg

If you get asserts in winedb about `wine/dlls/ntdll/unix/server.c` !status
then consider rebooting, is definitely helped once.


