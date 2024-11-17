# Debugging

The **mulle-objc** runtime has some facilities to help you understand runtime
problems. For memory problems (besides `MULLE_OBJC_TRACE_INSTANCE`)  have a look at [mulle-testallocator](//github.com/mulle-core/mulle-testallocator/blob/release/dox/API_TESTALLOCATOR.md#environment-variables).

## Warnings

Use the following environment variables to let the runtime warn you about
inconsistencies. You can set the environment variables to "YES" or "NO".


| Variable                               |  Function
|----------------------------------------|--------------------------------
| `MULLE_OBJC_WARN_ENABLED`              | Enables all the following warnings.
| &nbsp;                                 | &nbsp;
| `MULLE_OBJC_WARN_METHOD_TYPE`          | Warn if methods with identical names have different types. Example: `- (BOOL) | load` and `+ (void) load`.
| `MULLE_OBJC_WARN_PEDANTIC_METHODID_TYPE` | This is faster and complains more as it is just string comparing the | signatures.
| `MULLE_OBJC_WARN_PROTOCOLCLASS`        | Warn if a class does not fit the requirements to be a protocol class, but a | protocol of the same name exists.
| `MULLE_OBJC_WARN_STUCK_LOADABLE`       | Warn if classes or categories could not be integrated into the runtime class | system. This indicates a missing class or category. The warning appears, when the runtime is released (end of the | program). This is enabled by default currently.
| `MULLE_OBJC_WARN_CRASH`                | If set to `YES` a warning will force an `abort`
| `MULLE_OBJC_WARN_HANG`                 | If set to `YES` a warning will force a `mulle_objc_hang` | (`MULLE_OBJC_WARN_CRASH` had precedence though)

## Traces

Use the following environment variables to trace runtime operations. You can
set the environment variables to "YES" or "NO".


|  Variable                              |  Function
|----------------------------------------|--------------------------------
| `MULLE_OBJC_TRACE_CLASS_CACHE`         | Trace as the class cache is created and enlarged.
| `MULLE_OBJC_TRACE_METHOD_CACHE`        | Trace method caches as they are created and enlarged.
| `MULLE_OBJC_TRACE_METHOD_SEARCH`       | Trace the search for a methods implementation. This is a good way to learn | about the way Objective-C does inheritance.
| `MULLE_OBJC_TRACE_METHOD_CALL`         | Trace the calling of Objective-C methods, creates lots of output.
| &nbsp;                                 | &nbsp;
| `MULLE_OBJC_TRACE_ENABLED`             | Enables all the following traces.
| &nbsp;                                 | &nbsp;
| `MULLE_OBJC_TRACE_CATEGORY_ADD`        | Trace whenever a category is added to the runtime system.
| `MULLE_OBJC_TRACE_CLASS_ADD`           | Trace whenever a class is added to the runtime system.
| `MULLE_OBJC_TRACE_CLASS_FREE`          | Trace whenever a class is freed.
| `MULLE_OBJC_TRACE_DEPENDENCY`          | Trace whenever a class or category is queued up to be added to the runtime | system, when it's dependencies have not appeared yet.
| `MULLE_OBJC_TRACE_DUMP_RUNTIME`        | Periodically dump the runtime to tmp during loading.
| `MULLE_OBJC_TRACE_FASTCLASS_ADD`       | Trace whenever a "fast" class is added to the runtime system.
| `MULLE_OBJC_TRACE_HASHSTRINGS`         | Trace addition of hash strings
| `MULLE_OBJC_TRACE_INSTANCE`            | Trace instance allocation and deallocation
| `MULLE_OBJC_TRACE_INITIALIZE`          | Trace calls or non-calls of `+initialize` and +load
| `MULLE_OBJC_TRACE_LOADINFO`            | Trace the enqueing of loadinfos
| `MULLE_OBJC_TRACE_PROTOCOL_ADD`        | Trace whenever a protocol is added to the runtime system.
| `MULLE_OBJC_TRACE_STATE_BIT`           | Trace whenever a class state changes.
| `MULLE_OBJC_TRACE_STRING_ADDS`         | Trace whenever a constant string is added to the runtime system.
| `MULLE_OBJC_TRACE_TAGGED_POINTER`      | Trace whenever a class registers for tagged pointers (isa).
| `MULLE_OBJC_TRACE_UNIVERSE`            | Trace construction and destruction of universes

## Settings

|  Variable                              |  Function
|----------------------------------------|--------------------------------
| `MULLE_OBJC_PEDANTIC_EXIT`             | Force destruction of the universe at the end of the program run.


## Prints

Like a lesser variation of traces, either prints just once or modified a trace.


| Variable                               |  Function
|----------------------------------------|--------------------------------
| `MULLE_OBJC_PRINT_UNIVERSE_CONFIG`     | Print the version of the universe, maybe more in the future.
| `MULLE_OBJC_PRINT_ORIGIN`              | Print the owner of methodlists in loadinfo traces. This is enabled by default currently.


## Dumps

You can dump the runtime in HTML or [Graphviz](//www.graphviz.org/) format to
the filesystem. The Graphviz format is graphical whereas the HTML format is
text only.


### `mulle_objc_universe_htmldump_to_directory`

```
void mulle_objc_universe_htmldump_to_directory( struct _mulle_objc_universe *u,
                                                char *directory);
```

Use `mulle_objc_universe_htmldump_to_directory` to dump a universe to any
directory as HTML files. It will produce a lot of files, detailing all the
classes, protocols and so forth.


### `mulle_objc_universe_dotdump_to_directory`

```
void  mulle_objc_universe_dotdump_to_directory( struct _mulle_objc_universe *u,
                                                char *directory);
```

Use `mulle_objc_universe_dotdump_to_directory` to dump a universe into `.dot`
files. Then maybe convert the .dot files into PDF with
`dot -Tpdf debug.dot -o debug.pdf`.


# Debugging the mulle-objc runtime itself

Build the **mulle-objc** with `-DDEBUG` and do not define
`-DNDEBUG` to keep `assert` alive. This is the first major step to debugging
runtime problems. It usually makes little sense to develop with anything else
than a debugging runtime.

