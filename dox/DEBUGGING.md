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
`MULLE_OBJC_WARN_STUCK_LOADABLES`     | Warn if classes or categories could not be integrated into the runtime class system. This indicates a missing class or category.
`MULLE_OBJC_WARN_PROTOCOLCLASS`       | Warn if a class does not fit the requirements to be a protocol class, but a protocol of the same name exists

## Traces

Use the following environment variables to trace runtime operations. You can set the environment variables to "YES" or "NO".


 Variable                               |  Function
----------------------------------------|--------------------------------
`MULLE_OBJC_TRACE_METHOD_SEARCHES`      | Trace the search for a methods implementation. This is a good way to learn about the way Objective-C does inheritance.
`MULLE_OBJC_TRACE_METHOD_CALLS`         | Trace the calling of Objective-C methods. output.
&nbsp;                                  | &nbsp;
`MULLE_OBJC_TRACE_ENABLED`              | Enables all the following traces.
&nbsp;                                  | &nbsp;
`MULLE_OBJC_TRACE_CATEGORY_ADDS`        | Trace whenever a category is added to the runtime system.
`MULLE_OBJC_TRACE_CLASS_ADDS`           | Trace whenever a class is added to the runtime system.
`MULLE_OBJC_TRACE_CLASS_FREES`          | Trace whenever a class is freed.
`MULLE_OBJC_TRACE_DELAYED_CATEGORY_ADDS`| Trace whenever a category is queued up to be added to the runtime system, when it's class has not appeared yet.
`MULLE_OBJC_TRACE_DELAYED_CLASS_ADDS`   | Trace whenever a class is queued up to be added to the runtime system, when it's superclass(es) appears.
`MULLE_OBJC_TRACE_FASTCLASS_ADDS`       | Trace whenever a "fast" class is added to the runtime system.
`MULLE_OBJC_TRACE_INITIALIZE`           | Trace calls or non-calls of `+initialize`
`MULLE_OBJC_TRACE_LOAD_CALLS`           | Trace calls of `+load`, `+categoryDependencies`, `+classDependencies`
`MULLE_OBJC_TRACE_LOADINFO`             | Trace the enqueing of loadinfos
`MULLE_OBJC_TRACE_STRING_ADDS`          | Trace whenever a constant string is added to the runtime system.
`MULLE_OBJC_TRACE_TAGGED_POINTERS`      | Trace whenever a class registers for tagged pointers (isa).


## Dumps

You can dump the runtime in HTML or [Graphviz](//www.graphviz.org/) format to the filesystem. The Graphviz format is graphical and only suitable for
for very small runtime systems. The HTML format is generally more practical.


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

