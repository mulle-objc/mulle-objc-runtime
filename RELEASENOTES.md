### 0.9.1

Finally got around to a renaming. The runtime library is now,
mulle-objc-runtime and it's header is mulle-objc-runtime.h. I renamed the struct
mulle-objc-runtime to mulle-objc-universe. Though that's confusing in the short
term, I am not a big fan of overloaded definitions.

This has the benefit of freeing up the <mulle_objc/mulle_objc.h> header, that
should be part of MulleObjC.

`__MULLE_OBJC_TRT__` will be renamed to `__MULLE_OBJC_TLU__` in 1.0

It should now be possible to initialize a global universe from multiple threads.
It should not be necessary to set the global universe up from a single thread,
before starting other threads.

Only use `mulle_objc_get_or_create_universe` during loading. Makes it easier.

Also distribute mulle-objc-uniqueid.

Changed the coverage format to be compatible with mulle-objc-list. Use a more
all encompassing form of coverage check using a methoddescriptor bit.

Initialize the cache now before +initialize. To be able to affect the cache
size (and possible other stuff) there is a new callback in the universe
`will_init_cache`. The +initialize code has been reworked, so that superclass
+initializes are now guaranteed to be called before the subclass.

Made it a "C" cmake project

* modernize to mulle-configuration 2.0.0
* modernize CMakeDependencies.txt and CMakeLists.txt
* better MULLE_OBJC_TRACE_UNIVERSE output
* will_dealloc callback for universe deconstruction

### 0.8.5

* modernize project, beautify and fix dotdump a little

### 0.8.4

* Community release


### 0.8.3

* enlarged class state bit space for mulle-objc to 16 bits, which cramps
user and foundation to 8 bits each
* added cache csv dump fo future shenanigans


## 0.8.1

* introduced the _mulle_objc_protocol structure, which is now emitted by the
compiler. It's sole use is to detect hash collisions on protocolids, as unlikely
they maybe
* also there is now a central table for categories, also just to check for
collisions
* improved the search speed of conformsToProtocol:
* upped the LOAD version to 8
* renamed _size_of_ functions to _sizeof_ as its more c like
* renamed has_category/has_protocol and related functions to has_categoryid/has_protocolid,
because it is less confusing
* removed misleading mulle_objc_string_for_ivarid and friends (use class to get
the names)

## 0.7.1

> This version has changes mostly to the benefit of the mulle-lldb debugger

* moved some more call functions into "...call.c" (duh)
* moved version and new "path" up for easier debugging and easier version
checks its kinda useful if version is at a fixed offset
* `__get_or_create_objc_runtime` has been renamed to `__get_or_create_mulle_objc_runtime`
for clarity in the debugger and multi-universe code.
* moved forward into a fixed position in the class structure for a future
debugger
* the forward function is now fixed to be named as
`__forward_mulle_objc_object_call` as a benefit to the debugger. Let's put it
this way, you can still forward on a per class basis, but the debugger won't
pick up on it. Which isn't a problem just not as gainly looking in the
stacktrace'

## 0.6.1

* coverage file "append" to existing coverage files (seems more useful)
* added short-name dump routines to dump to working directory
* added _mulle_objc_searchresult to _mulle_objc_class_search_method, so one can
find the location a method is stored
* shortened the names of the dump sources
* renamed various "get_id" functions to get_<type>id" for consistency
* added coverage csv dumpers for future optimization benefits
* renamed ..._CACHE_INITIALIZED to ..._CACHE_READY, because it confused me
* when doing a dotdump of everything, don't write out each class filename
* improved protocol class detection, so now protocolclasses can conform
to protocols (as long as they aren't protocolclasses)
* added MULLE_OBJC_TRACE_DEPENDENCIES
* added MULLE_OBJC_TRACE_STATE_BITS
* +classDependencies and +categoryDependencies don't exist anymore. They are
replaced by +dependencies, which combines them. To specify a dependency on
a class do `{ @selector( Class), 0 }`, on a category do
`{ @selector( Class), @selector( Category) }`

```
+ (struct _mulle_objc_dependency *) dependencies
```


## 0.5.1

* this universe needs the mulle-clang compiler, based on clang 4.0.0
* significantly improved the graphviz and html dumper to be more useable for bigger
universe environments.

## 0.4.1

* this universe needs the mulle-clang compiler, based on clang 4.0.0
* the universe load structures now have their own independent versioning
called MULLE_OBJC_RUNTIME_LOAD_VERSION
* the class now contains a list of categoryids, which can be queried at universe
* the class loader will check if a class implements +classDependencies,
this allows proper sequenced +loads of classes. the category loader will check
if a category implements +categoryDependencies. this allows proper sequenced
+loads of categories and their order in the  methodlists. To make this really
happen though, the loading of classes and categories had to be serialized with lock!
* Renamed `mulle_vararg_count_objects` and `mulle_vararg_next_object` to
`mulle_vararg_count_ids` and  `mulle_vararg_next_id`. `mulle_vararg_next_object`
now resides in MulleObjC.
* new trace flag `MULLE_OBJC_TRACE_LOAD_CALLS` traces `+load`, `+initialize` and
`+categoryDependencies` calls.
* With mulle_objc_check_runtimewaitqueues you can see if clases or categories are "stuck" waiting for dependencies
* `mulle_objc_object_get_retaincount` now returns a human readable number.
Ask the header for the raw value (if its not a tagged pointer)

#### New Environment Variables

It is preferred to set the environment variables with "YES" or "NO" now.

> Usually you rather undefined an environment variable, then set it to "NO"

Variable                          | Description
----------------------------------|-----------------------
`MULLE_OBJC_TRACE_LOADINFO`       | more detailed output of the loadinfo
`MULLE_OBJC_TRACE_CATEGORY_ADDS`  | trace universe category additions
`MULLE_OBJC_TRACE_PROTOCOL_ADDS`  | trace universe protocol additions
`MULLE_OBJC_WARN_STUCK_LOADABLES` | replaces ..._WARN_NOTLOADED_CLASSES and .._CATEGORIES


## 0.3.1

* renamed `MULLE_OBJC_HAVE_THREAD_LOCAL_RUNTIME` to `__MULLE_OBJC_TRT__` and
added compiler support for it
* added `__MULLE_OBJC_NO_TRT__`, `__MULLE_OBJC_NO_TPS__`, `__MULLE_OBJC_NO_AAM__`,
this helps to find problems, when mulle-objc code is compiled with a C compiler
only
* renamed `mulle_objc_object_get_taggedpointer_index` to `mulle_objc_object_get_taggedpointer_index`
* new trace flag `MULLE_OBJC_TRACE_RUNTIME_CONFIG`
* `CMakeLists.txt` sets `__MULLE_OBJC_TPS__` and `__MULLE_OBJC_NO_TRT__` by
default. Since the universe is  built by the standard host compiler, this
matches the current default of the  mulle-clang compiler
* improved and more lenient thread local compatibility check on load
* the pass-through for foundation code w/o version has been removed
* `MULLE_OBJC_USER_VERSION_MAJOR` and friends shortened to `USER_VERSION_MAJOR`
* `MULLE_OBJC_FOUNDATION_VERSION_MAJOR` and friends shortened to `FOUNDATION_VERSION_MAJOR`
* `mulle_vararg_next_id` is the old `mulle_vararg_next_object`.
`mulle_vararg_next_object` now accepts a type.

## 0.2.3

* improved super calls


>>>>>>> Stashed changes
## 0.2.1

* add support for char5-encoding check
* rename some load struct fields
* dependency check on "class" protocolids added
* renamed MULLE_OBJC_HAVE_THREAD_LOCAL_RUNTIME to MULLE_OBJC_THREAD_LOCAL_RUNTIME
* universe checks for mismatch between global and thread local universe code

## 0.1.7

* Merge in community version changes


## 0.1.6

* Compile fix for linux / gcc


## 0.1.4

* Improve documentation


## 0.1.2

Community Release

## 0.1.1

Merciful Release
