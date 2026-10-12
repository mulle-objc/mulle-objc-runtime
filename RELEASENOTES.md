### 0.30.1





* Document second-pass ``@method_implementation`` alias resolution so targets from adopted mixins or superclasses are found
* Document relaxed increment / `acq_rel` decrement retain-count atomics
* Note ``_mulle_objc_universe_get_foundationallocator`` may be NULL (no longer ``MULLE_C_NONNULL_RETURN`)`
* Update version to 0.30.1 and dependency ranges (mulle-core `[0.10.0, 0.11.0)`, mulle-core-all-load `[0.9.0, 0.10.0)`)

* change version to 0.31



feature: add Windows UTF-8 executable manifest support and optimize retaincount atomics

* Add `EXE_UTF8_MANIFEST` option to embed a UTF-8 activeCodePage application manifest into executables, so argv and environment arrive as UTF-8 on Windows 10 1903+
* Point the library/executable link target at the compile target during the compile-only phase, preventing unguarded ``target_link_options()`` calls from failing
* Use relaxed atomic increment for retain count claims and document the `acq_rel` requirement for decrement
* Correct comment referencing the mulle-objc-list project

* split tools off into mulle-objc-tools

## 0.30.0



* new standardimpmask fast path tracks which method families still use a class's default implementation, speeding up dispatch and cache invalidation
* class load wait queues and the universe registry now use lock-free concurrent hashtables, allowing multi-threaded removal and relaxing single-threaded assumptions
* new ``mulle_objc_universe_set_stacktrace_callback`` API lets applications install a custom stacktrace callback for runtime failure, inconsistency, and not-found aborts
* class posing `(`patch_infraclass`)` now re-registers the posed-away superclass under a unique `%`-prefixed class id, fixing a teardown leak; it now requires a superclass and refuses fast classes
* threadinfo gains a separate `objectallocator` so per-thread instance reuse works while infrastructure allocations stay untracked by the test allocator



* added missing copyright headers in generic and standalone source files
* fixed incorrect filenames in several header comment blocks
* updated README with documentation and guide links
* added trailing newline at end of retain-release source file
