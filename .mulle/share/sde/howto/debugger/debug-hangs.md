# mulle-sde Debug Hangs Guidelines
<!-- Keywords: debug, debugging, hang, deadlock, freeze, stuck, stacktrace -->

## Workflow

**When a test or process hangs: get all thread stacks first, fix second.**

Never re-run the test repeatedly grepping for log lines. A hang means threads
are blocked. The stacks tell you exactly where and why.

### 1. Run the process in the background and let it hang

```bash
mulle-sde run [executable] -- [arguments] &
HUNG_PID=$!
sleep 5   # give it time to hang
```

Or with the test runner, note the PID from the timeout output.

### 2. Attach gdb and dump all thread stacks

```bash
gdb -p $HUNG_PID -batch -ex "thread apply all bt" -ex "quit" 2>/dev/null
```

### 3. Read the stacks

Look for threads blocked on:
- `pthread_mutex_lock` / `pthread_cond_wait` — lock contention or deadlock
- `mulleLockWhenCondition:` / `mulleJoin` — condition lock deadlock
- `os_waitEventsWithTimeout:` — event loop blocked (usually fine, check other threads)

A deadlock shows two or more threads each waiting for a lock the other holds.
Identify the lock, find who holds it, trace back to the call that acquired it
without releasing.

### 4. Fix the actual problem

Do not guess. The stacks show the exact call site. Fix what they show.

## Interactive debugging

```bash
gdb ./executable
(gdb) run
# wait for hang, then Ctrl-C
(gdb) thread apply all bt
(gdb) thread 2
(gdb) bt
(gdb) frame 3
(gdb) print variable
(gdb) info locals
```

## Full logging before attaching gdb

If the stacks alone are not enough context, enable full logging first.
With MulleUI apps this will be:

```bash
mulle-sde -DUIDebuggingFlags=0x03FFFFFF test run --timeout 12 <test> > /tmp/hang.log 2>&1
```

for Foundation `-DNSDebugEnabled=YES`. There is a plentitude of other debugging
and trace facilities.


Then study `/tmp/hang.log` — look at the last lines before the timeout to see
what the main thread and window thread were doing just before the hang.

## More

```bash
mulle-sde debug help
```
