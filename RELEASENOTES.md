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
