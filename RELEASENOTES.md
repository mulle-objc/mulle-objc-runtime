## 0.6.1

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

* this runtime needs the mulle-clang compiler, based on clang 4.0.0
* significantly improved the graphviz and html dumper to be more useable for bigger
runtime environments. 

## 0.4.1

* this runtime needs the mulle-clang compiler, based on clang 4.0.0
* the runtime load structures now have their own independent versioning
called MULLE_OBJC_RUNTIME_LOAD_VERSION
* the class now contains a list of categoryids, which can be queried at runtime
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
`MULLE_OBJC_TRACE_LOADINFO`       | more detailed output of the loadinfo
`MULLE_OBJC_TRACE_CATEGORY_ADDS`  | trace runtime category additions
`MULLE_OBJC_TRACE_PROTOCOL_ADDS`  | trace runtime protocol additions
`MULLE_OBJC_WARN_STUCK_LOADABLES` | replaces ..._WARN_NOTLOADED_CLASSES and .._CATEGORIES


## 0.3.1

* renamed `MULLE_OBJC_HAVE_THREAD_LOCAL_RUNTIME` to `__MULLE_OBJC_TRT__` and
added compiler support for it
* added `__MULLE_OBJC_NO_TRT__`, `__MULLE_OBJC_NO_TPS__`, `__MULLE_OBJC_NO_AAM__`,
this helps to find problems, when mulle-objc code is compiled with a C compiler
only
* renamed `mulle_objc_object_get_taggedpointer_index` to `mulle_objc_object_get_taggedpointer_index`
* new trace flag `MULLE_OBJC_TRACE_RUNTIME_CONFIG`
* `CMakeLists.txt` sets `__MULLE_OBJC_TPS__` and `__MULLE_OBJC_NO_TRT__` by 
default. Since the runtime is  built by the standard host compiler, this 
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
* runtime checks for mismatch between global and thread local runtime code

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

