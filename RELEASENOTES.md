## 0.28.0








feature: enforce category +dependencies for overriding methods

* Runtime now checks that categories which override existing methods implement +dependencies and list the overridden category; missing or incorrect declarations produce clear error messages and cause the universe to be marked inconsistent (**BREAKING**)
* Adds a special MulleObjCDeps class id to collect/skip dependency checks and includes tests covering proper, missing, wrong, and chained category dependency scenarios

















feature: add mulle-metaabi-call header and meta-ABI call tests

* add public header src/mulle-metaabi-call.h to expose meta-ABI call helpers
* add extensive compiler/runtime tests validating meta-ABI calling conventions and call sequence behavior
* rename internal cache field from `thread` to ``thread_id`` (potential **BREAKING** for code that accessed cache internals)


feature: bump ``MULLE_OBJC_RUNTIME_LOAD_VERSION`` to 19, requiring compiler 21.1.8.7+

* **BREAKING**: ``MULLE_OBJC_RUNTIME_LOAD_VERSION`` bumped to 19 — requires mulle-clang compiler 21.1.8.7 or later
* fix inverted condition in ``_mulle_metaabi_get_metaabiparamtype`` that misidentified function pointers (`^?`) as regular pointers
* add ``__mulle_objc_universe_get_taggedpointerinfraclass`` accessor that triggers DLL loader for missing tagged pointer classes on Windows
* add ``_mulle_objc_universe_tps_class_is_missing`` callback for Windows tagged pointer class resolution via DLL loader
* deduplicate DLL loader trace messages using a once-flag across class/method/TPS missing callbacks
* add retry loop in universe status check to allow DLL loader to resolve pending classes on Windows
* expose ``mulle_objc_universe_lookup_descriptor_nofail`` and ``_mulle_objc_universe_walk_hashmap`` with ``MULLE_OBJC_RUNTIME_GLOBAL``
* promote ``_mulle_objc_classpair_add_uniqueidarray_ids`` from inline forward declaration to proper global declaration
* improve debug output: include env var names ``MULLE_OBJC_WARN_HANG`` and ``MULLE_OBJC_WARN_CRASH`` in messages





feature: lazy-load sibling Objective-C DLLs on Win32 missing class/method lookups

* Windows runtimes can now attempt to load nearby Objective-C DLLs when a class or method can’t be resolved (primarily to simplify test executables)
* non-delegating infraclass lookup now uses the fast-class table when available, avoiding side effects during dependency/loadinfo resolution
* **BREAKING**: ``_mulle_objc_universe`` callback wiring changed (`callbacks` → `callback`; ``class_is_missing`/`method_is_missing`` now return “nochange” status to control retry)
