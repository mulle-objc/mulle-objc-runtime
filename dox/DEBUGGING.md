# Debugging

The **mulle-objc** runtime has some facilities to help you understand runtime
problems.

## Warnings

Use the following environment variables to let the runtime warn you about
inconsistencies. You can set the environment variables to "YES" or "NO".


Variable                              |  Function
------------------------------------- | --------------------------------
`MULLE_OBJC_WARN_ENABLED`             | Enables all the following warnings.
&nbsp;                                | &nbsp;
`MULLE_OBJC_WARN_METHODID_TYPES`      | Warn if methods with identical names have different types. Example: `- (BOOL) load` and `+ (void) load`.
`MULLE_OBJC_WARN_PEDANTIC_METHODID_TYPES` | This is faster and complains more as it is just string comparing the signatures.
`MULLE_OBJC_WARN_PROTOCOLCLASS`       | Warn if a class does not fit the requirements to be a protocol class, but a protocol of the same name exists.
`MULLE_OBJC_WARN_STUCK_LOADABLES`     | Warn if classes or categories could not be integrated into the runtime class system. This indicates a missing class or category. The warning appears, when the runtime is released (end of the program). This is enabled by default currently.

## Traces

Use the following environment variables to trace runtime operations. You can set the environment variables to "YES" or "NO".


 Variable                               |  Function
----------------------------------------|--------------------------------
`MULLE_OBJC_TRACE_CLASS_CACHE`          | Trace as the class cache is created and enlarged.
`MULLE_OBJC_TRACE_METHOD_CACHES`        | Trace method caches as they are created and enlarged.
`MULLE_OBJC_TRACE_METHOD_SEARCHES`      | Trace the search for a methods implementation. This is a good way to learn about the way Objective-C does inheritance.
`MULLE_OBJC_TRACE_METHOD_CALLS`         | Trace the calling of Objective-C methods. output.
&nbsp;                                  | &nbsp;
`MULLE_OBJC_TRACE_ENABLED`              | Enables all the following traces.
&nbsp;                                  | &nbsp;
`MULLE_OBJC_TRACE_CATEGORY_ADDS`        | Trace whenever a category is added to the runtime system.
`MULLE_OBJC_TRACE_CLASS_ADDS`           | Trace whenever a class is added to the runtime system.
`MULLE_OBJC_TRACE_CLASS_FREES`          | Trace whenever a class is freed.
`MULLE_OBJC_TRACE_DEPENDENCIES`         | Trace whenever a class or category is queued up to be added to the runtime system, when it's dependencies have not appeared yet.
`MULLE_OBJC_TRACE_DUMP_RUNTIME`         | Periodically dump the runtime to tmp during loading.
`MULLE_OBJC_TRACE_FASTCLASS_ADDS`       | Trace whenever a "fast" class is added to the runtime system.
`MULLE_OBJC_TRACE_INITIALIZE`           | Trace calls or non-calls of `+initialize`
`MULLE_OBJC_TRACE_LOAD_CALLS`           | Trace calls of `+load`, `+categoryDependencies`, `+classDependencies`
`MULLE_OBJC_TRACE_LOADINFO`             | Trace the enqueing of loadinfos
`MULLE_OBJC_TRACE_PROTOCOL_ADDS`        | Trace whenever a protocol is added to the runtime system.
`MULLE_OBJC_TRACE_STATE_BITS`           | Trace whenever a class state changes. `MULLE_OBJC_TRACE_STRING_ADDS`          | Trace whenever a constant string is added to the runtime system.
`MULLE_OBJC_TRACE_TAGGED_POINTERS`      | Trace whenever a class registers for tagged pointers (isa).


## Prints

Like a lesser variation of traces, either prints just once or modified a trace.


 Variable                               |  Function
----------------------------------------|--------------------------------
`MULLE_OBJC_PRINT_RUNTIME_CONFIG`       | Print the version of the runtime, maybe more in the future.
`MULLE_OBJC_PRINT_ORIGIN`               | Print the owner of methodlists in loadinfo traces. This is enabled by default currently.


## Dumps

You can dump the runtime in HTML or [Graphviz](//www.graphviz.org/) format to the filesystem. The Graphviz format is graphical whereas zhe HTML format is text only.


### `mulle_objc_htmldump_runtime_to_tmp`

```
void   mulle_objc_htmldump_runtime_to_tmp( void);
```

Use `mulle_objc_htmldump_runtime_to_tmp` to dump the current runtime
system to "/tmp/" as HTML files. It will produce a lot of files.


### `mulle_objc_dotdump_runtime_to_tmp`

```
void   mulle_objc_dotdump_runtime_to_tmp( void)
```

Use `mulle_objc_dotdump_runtime_to_tmp` to dump the current runtime
system into a `.dot` file. Then maybe convert the .dot file into PDF with
`dot -Tpdf debug.dot -o debug.pdf`.


### `mulle_objc_dotdump_classname_to_tmp`

```
void   mulle_objc_dotdump_classname_to_tmp( char *s)
```

Use `mulle_objc_dotdump_classname_to_tmp` to dump a class into a `.dot` file. Then maybe convert the .dot file into SVG with `dot -Tsvg debug.dot -o debug.dvg`.



# Debugging the mulle-objc runtime itself

Build the **mulle-objc** with `-DDEBUG` and do not define
`-DNDEBUG` to keep `assert` alive. This is the first major step to debugging
runtime problems. It usually makes little sense to develop with anything else
than a debugging runtime.

