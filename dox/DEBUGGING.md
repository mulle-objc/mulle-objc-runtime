# Debugging

The **mulle-objc** runtime has some facilities to help you understand runtime
problems.

## Warnings

Use the following environment variables to let the runtime warn you about
inconsistencies:

Variable                              |  Function
------------------------------------- | --------------------------------
`MULLE_OBJC_WARN_ENABLED`             | Enables all the following warnings.
&nbsp;                                | &nbsp;
`MULLE_OBJC_WARN_METHODID_TYPES`      | Warn if methods with identical names have different types. Example: `- (BOOL) load` and `+ (void) load.
`MULLE_OBJC_WARN_NOTLOADED_CATEGORIES`| Warn if categories could not be integrated into the runtime class system. This indicates a missing class.
`MULLE_OBJC_WARN_NOTLOADED_CLASSES`   | Warn if classes could not be integrated into the runtime class system. This indicates a missing superclass.
`MULLE_OBJC_WARN_PROTOCOL_CLASS`      | Warn if a class does not fit the requirements to be a protocol class, but a protocol of the same name exists

## Traces

Use the following environment variables to trace runtime operations. If the
environment variable exists, it counts as set.

 Variable                               |  Function
----------------------------------------|--------------------------------
`MULLE_OBJC_TRACE_ENABLED`              | Enables all the following traces, except `MULLE_OBJC_TRACE_METHOD_SEARCHES` and `MULLE_OBJC_TRACE_METHOD_CALLS`
&nbsp;                                  | &nbsp;
`MULLE_OBJC_TRACE_CATEGORY_ADDS`        | Trace whenever a category is added to the runtime system.
`MULLE_OBJC_TRACE_CLASS_ADDS`           | Trace whenever a class is added to the runtime system.
`MULLE_OBJC_TRACE_CLASS_FREES`          | Trace whenever a class is freed.
`MULLE_OBJC_TRACE_DELAYED_CATEGORY_ADDS`| Trace whenever a category is queued up to be added to the runtime system, when it's class has not appeared yet.
`MULLE_OBJC_TRACE_DELAYED_CLASS_ADDS`   | Trace whenever a class is queued up to be added to the runtime system, when it's superclass appears.
`MULLE_OBJC_TRACE_FASTCLASS_ADDS`       | Trace whenever a "fast" class is added to the runtime system.
`MULLE_OBJC_TRACE_LOADINFO`             | Trace the enqueing of loadinfos
`MULLE_OBJC_TRACE_METHOD_SEARCHES`      | Trace the search for a methods implementation. This is a good way to learn about the way Objective-C does inheritance.
`MULLE_OBJC_TRACE_METHOD_CALLS`         | Trace the calling of Objective-C methods. output.
`MULLE_OBJC_TRACE_STRING_ADDS`          | Trace whenever a constant string is added to the runtime system.
`MULLE_OBJC_TRACE_TAGGED_POINTERS`      | Trace whenever a class registers for tagged pointers (isa).


## Dumps

You can dump the runtime in HTML or [Graphviz](//www.graphviz.org/) format to the filesystem. The
Graphviz format is graphical and only suitable for
for very small runtime systems. The HTML format is generally more practical.


### `mulle_objc_dump_runtime_as_html_to_tmp`

```
void   mulle_objc_dump_runtime_as_html_to_directory( char *directory);
```

Use `mulle_objc_dump_runtime_as_html_to_directory` to dump the current runtime
system to a directory. It will produce a lot of files.


### `mulle_objc_runtime_dump_graphviz_to_file`

```
void   mulle_objc_runtime_dump_graphviz_to_file( char *filename)
```

Use `mulle_objc_runtime_dump_graphviz_to_file` to dump the current runtime
system into a `.dot` file. Then convert the .dot file into PDF with
`dot -Tpdf debug.dot -o debug.pdf`.



# Debugging the mulle-objc runtime itself

Build the **mulle-objc** with `-DDEBUG` and do not define
`-DNDEBUG` to keep `assert` alive. This is the first major step to debugging
runtime problems. It usually makes little sense to develop with anything else
than a debugging runtime.

