## GDB stacktrace for AI
<!-- Keywords: gdb, stacktrace, crash -->

To debug an executable with GDB and get a backtrace on a crash, use the
following command:

```bash
executable="`mulle-sde product`"
gdb --batch -ex 'run' -ex 'bt' -ex 'set confirm off' -ex 'quit' --args "${executable}"
```

If you have multiple executables built (like in a demo project), use
`mulle-sde product <productname>`.

For tests you will substitute the `mulle-sde product` call with the executable
path you know.

In the same manner you can also set breakpoints and inspect values. Do not
forget to add `-ex 'set confirm off' -ex 'quit'` or you will get stuck.
