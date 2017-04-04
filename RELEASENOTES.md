## 0.4.1

* this runtime needs the mulle-clang compiler, based on clang 4.0.0
* the runtime load structures now have their own independent versioning
called MULLE_OBJC_RUNTIME_LOAD_VERSION
* the class now contains a list of categoryids, which can be queried at runtime
* the category loader will check if a category implements +categoryDependencies,
this allows proper sequenced +loads of categories and their order in the 
methodlists. To make this really happen though, the loading of classes and
categories had to be serialized with lock!
* Renamed `mulle_vararg_count_objects` and `mulle_vararg_next_object` to
`mulle_vararg_count_ids` and  `mulle_vararg_next_id`. `mulle_vararg_next_object` 
now resides in MulleObjC.
* new trace flag `MULLE_OBJC_TRACE_LOAD_CALLS` traces `+load`, `+initialize` and
`+categoryDependencies` calls.

#### New Environment Variables

Variable                          | Description
----------------------------------|-----------------------
`MULLE_OBJC_TRACE_LOADINFO`       | more detailed output of the loadinfo
`MULLE_OBJC_TRACE_CATEGORY_ADDS`  | trace runtime category additions
`MULLE_OBJC_TRACE_PROTOCOL_ADDS`  | trace runtime protocol additions


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

