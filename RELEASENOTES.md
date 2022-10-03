### 0.21.2

* correct all version numbers

### 0.21.1

* fix bug in `_mulle_objc_class_lookup_implementation_mode`
This bugfix may make lookup functions slower, so regular execution should not be affected. Those lookups which are now slower can be easily fixed though. Just complain :)

## 0.21.0

* fix `mulle_objc_signature_supply_typeinfo` to not suppress supplier
* fix runtime initializing classes on shutdown needlessly
* rename `_mulle_objc_searcharguments_superinit` and its ilk to `_mulle_objc_searcharguments_init_super` for consistency with other functions
* change GLOBALs for Windows
* rename `_mulle_objc_object_cacheonlylookup_implementation` to `_mulle_objc_object_lookup_implementation_cacheonly`


## 0.20.0

* moved debug support out into mulle-objc-debug, which reduces the size of the runtime proper by 25%
* redid method call functions
* changed class header ivar order, removed some unused ivars
* some code refactorings


## 0.19.0

* slow -release support when you need to suppress inlined retain counts and maintain your own
* main thread may now wait on background threads before exit, if desired
* instance meta header (retaincount, isa) is not fixed size anymore
* added ``mulle_objc_symbolizer_done``

## 0.18.0

* rename runtime functions, e.g. `_mulle_objc_object_partialinlinesupercall` to `mulle_objc_object_supercall_inline_partial` for consistency
* moved hash code shared by other projects to mulle-data
* reorder parameters of ``mulle_objc_class_trace_call`` for consistency
* give instantiate its own placeholder, this makes +object work with classclusters as well
* add ``mulle_objc_searchresult_get_categoryid`` and ``mulle_objc_searchresult_get_classid``
* add ``mulle_objc_universe_lookup_infraclass`` for consistency
* graphviz methodlists are now sorted
* you can now use  to dump the class hierarchy for a specific infraclass or metaclass
* renamed ``_mulle_objc_signature_pedantic_compare`` `to`_mulle_objc_signature_compare`` and  ``_mulle_objc_signature_compare`` to ``_mulle_objc_signature_compare_lenient`.` The pedantic compare is now also the default (as its faster)
* there is now a map that lists selectors for which selectors with varying types exist. Use ``_mulle_objc_universe_lookup_varyingsignaturedescriptor`` to query it
* `mulle_objc_metabi` is now `mulle_metaabi_` possibly to be moved outside of mulle-objc-runtime in the future.
* there is now a metaabi reader and a metaabi writer to support interpreters
* replace `object` for `instantiate` in the fastmethods table
* foundation allocator is now a pointer, which is more convenient
* reorganized some structs to support mulle-gdb easier
* now has some special case test ouputs for i686
* improved signature comparison, now ignores return value by default

### 0.17.1

* new mulle-sde project structure

## 0.17.0

* experimental stackdepth indent for method trace (not that satisfactory, but occasionally useful)
* up the version to 0.17, load version stays at 16
* redid class +initialize code, it now uses a per-classpair mutex for simplicity
* renamed some `_mulle_objc_object_...` functions to `_mulle_objc_instance_` when it is clear, that object can't be a class
* improved trace capabilities for threaded mulle-objc code
* redid +initialize, instead of using implicit locks with atomic bits and `thread_yield` now a per-class mutex is used
* clarified the signature offset to be only useful for invocations
* The initializing code, now puts caches on infra an meta classes immediately
* added signature enumeration, clarified that offsets are for NSInvocation only
* added `MULLE_OBJC_TRACE_THREAD`
* threadinfo now has destructors or foundation and userspace
* reduced `S_MULLE_OBJC_UNIVERSE_FOUNDATION_SPACE` to 512 but added some userspace to universe
* support for dynamic properties added (auto-create selectors for getter,setter,adder,remover)
* improved method family bits to discern accessors from regular methods
* add ``_mulle_objc_object_is_finalized`` method
* ``_C_BOOL`` is now used to serialize BOOL (not! ``_Bool`)`
* type modifiers like ``_C_CONST`` or ``_C_INOUT`` are no longer encoded/decoed in mulle-objc
* runtime may wait for non-main threads to complete before exiting
* which is not just beneficial for tests, but makes thread writing much easier
* preserve errno over method calls (inluding +initialize)


### 0.16.1

* fix `debug.warn.stuck_loadable` which is executed too late in the universe teardown

## 0.16.0

* start of special gdb support
* fix `MULLE_OBJC_COVERAGE` cache fill bug
* remove retain/release from fastcalls (superflous)
* use mulle-atinit und mulle-atexit now
* support new mulle-clang 9.0 property attributes "container" and "observable"
* improved universe crunch, will now notify classes with willfinalize
* method search can now search for a specific IMP
* category names are now (properly) available via methodlists and API
* classes gain the +finalize method (and +willFinalize)
* fix a KVC bug
* startup code has been moved to its own library mulle-objc-runtime-startup
* `MULLE_OBJC_TRACE_LOAD_CALL` is no more, its part of `MULLE_OBJC_TRACE_DEPENDENCY`
* `mulle_objc_..._current_thread` functions and related are now named `mulle_objc_thread_...`


### 0.15.3

* improved symbolification of stacktrace

### 0.15.2

* messed up the version number in 0.15.1

### 0.15.1

* improved find_library code for mulle-atexit
* fix RPATH for executables on OS X

## 0.15.0

* avoid setting errno during messages
* improved support for lldb
* the debughashname is now hashstring and its always loaded
* make loadclass accessible from classpair, makes origin superflous
* Inherit inheritance from superclass


### 0.14.1

* modernized mulle-sde with .mulle folder

## 0.14.0

* for release 7.0.0.0 of the mulle-clang compiler
* the mulle-objc runtime can now handle multiple universes
* most if not all of the runtime convenience functions had to be purged or renamed
* the threadlocal runtime is gone, superseded by universes with names
* there is now a clean distinction between ObjC exceptions and runtime errors called fails
* the compiler now needs to emit universeids to properly lookup classes
* for class lookups inside a method, there should be a considerable speedup
* improved universe trace facility
* moved debug conveniences to MulleObjC
* RENAMED: too many methods to list
* did some work on the big bang, for MulleObjC benefit
* use new verbs threadset and threadget and rename some functions
* add minimal.h for MulleObjC C includes
* BREAKING: renamed `_mulle_objc_universe_get_allocator` to `_mulle_objc_universe_get_foundation_allocator`
* RENAMED: `mulle_objc_inlined_get_universe` to `mulle_objc_inlineget_universe`
* RENAMED: `_mulle_objc_cachepivot_atomic_get_cache` to `_mulle_objc_cachepivot_atomicget_cache`
* RENAMED: `mulle_objc_object_copy` to `mulle_objc_object_call_copy`
* RENAMED: `mulle_objc_object_autorelease` to `mulle_objc_object_call_autorelease`
* RENAMED: `mulle_objc_object_copy` to `mulle_objc_object_call_copy`
* ADDED: `mulle_objc_invalidate_class_caches`
* BREAKING: `mulle_objc_infraclass_alloc_instance` now has no allocator parameter, use `__mulle_objc_infraclass_alloc_instance` if you must
* ADDED: `_mulle_objc_class_lookup_method` to search in a class without recursion for a method
* ADDED: `_mulle_objc_infraclass_get_allocator`
* BREAKING: Load version set to 13, as the ivar order in methods changed
* ADDED: `_mulle_objc_instance_get_allocator,` get the allocator of the class
* signature is now aware of function pointers and blocks
* BREAKING: `mulle_objc_signature_next_type` will now always return NULL if there is no next type, and not an empty string.
* mulle-objc-uniqueid can now deal with multiple arguments and can output CSV
* changes for mulle-lldb 6.0.0.5
* improve trace output for methods
* fix mingw, update sde
* fix DEBUGGING.md variable names


## 0.13.0

* moved compiler test-suite back from MulleObjC
* migrated to mulle-sde
* for release 6.0.0 of the mulle-clang compiler


## 0.12.1

* _MULLE_OBJC_CLASS_HAS_CLEARABLE_PROPERTY replaces _MULLE_OBJC_CLASS_HAS_RELEASABLE_PROPERTY
* _mulle_objc_object_infiniteretain_noatomic replaces _mulle_objc_object_nonatomic_infinite_retain
* Adapt runtime to __MULLE_OBJC_FCS__ to compile without fast methods
* adapt search so that we a root class inheriting from protocolclasses, also inherits the infraclass methods from the first protocolclass
* adapted call functions to the new way of calling super, where the classid of the calling class is passed (no longer the superclassid)
* add MULLE_OBJC_CLASS_HAS_RELEASABLE_PROPERTY bit for the sake of benchmarking
* add searchcache functionality to the runtime
* added _mulle_objc_signature_skip_extendedtypeinfo
* expect propertyclasses now in declaration order from compiler
* fixes for windows
* follow mulle-configuration 3.1 changes and move .travis.yml to trusty
* improve contents of signature typeinfo
* improve dotdump method cache
* improve superfunctions some more by inlining first stage also. Put everything into the methodcache
* increase fast class cache size to 64
* load version 12
* mingw fixes and hacks
* MULLE_OBJC_PRINT_RUNTIME_CONFIG renamed to MULLE_OBJC_PRINT_UNIVERSE_CONFIG
* new super call with its own cache
* property gains an ivarid
* reduce initial method cache size to 4
* rename _mulle_objc_uniqueid_is_sane to mulle_objc_uniqueid_is_sane
* super struct now gains the selector name for introspection. The selector is now compatible to @selector
* support newer mulle-tests
* up the load version, because of function renaming
* use a tuned fnv1a hash for better cache utilization


### 0.11.3

* follow mulle-configuration 3.1 changes and move .travis.yml to trusty

## 0.11.1

* fixes for windows
* mingw fixes and hacks
* use a tuned fnv1a hash for better cache utilization
* improve dotdump method cache
* improve contents of signature typeinfo
* reduce initial method cache size to 4
* increase fast class cache size to 64
* add MULLE_OBJC_CLASS_HAS_RELEASABLE_PROPERTY bit for the sake of benchmarking
* improve superfunctions some more by inlining first stage also. Put everything into the methodcache
* super struct now gains the selector name for introspection. The selector is now compatible to @selector
* Adapt runtime to __MULLE_OBJC_FCS__ to compile without fast methods
* expect propertyclasses now in declaration order from compiler
* adapt search so that a root class inheriting from protocolclasses, also inherits the infraclass methods from the first protocolclass
* adapted call functions to the new way of calling super, where the classid of the calling class is passed (no longer the superclassid)
* add searchcache functionality to the runtime
* up the load version, because of function renaming


## 0.10.1

Shortened `mulle_objc_methoddescriptor_t` to `mulle_objc_descriptor_t`
Shortened `mulle_objc_methodimplementation_t` to `mulle_objc_implementation_t`
Added new `struct _mulle_objc_super` to support even faster and better super
calls (and **overridden** calls in the future)
Renamed a lot of functions to make them easier to understand what they do.

Went a bit nuts on verbing stuff, instead of using underscores. So for instance
all `_get_or_lookup` functions are now called `fastlookup`.


## 0.9.1

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

Only use `mulle_objc_global_register_universe` during loading. Makes it easier.

Also distribute mulle-objc-uniqueid.

Changed the coverage format to be compatible with mulle-objc-list. Use a more
all encompassing form of coverage check using a descriptor bit.

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
* removed misleading mulle_objc_describe_ivarid and friends (use class to get
the names)

## 0.7.1

> This version has changes mostly to the benefit of the mulle-lldb debugger

* moved some more call functions into "...call.c" (duh)
* moved version and new "path" up for easier debugging and easier version
checks its kinda useful if version is at a fixed offset
* `__get_or_create_objc_runtime` has been renamed to `__register_mulle_objc_universe`
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
* added MULLE_OBJC_TRACE_STATE_BIT
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
* renamed `mulle_objc_object_get_taggedpointerindex` to `mulle_objc_object_get_taggedpointerindex`
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
