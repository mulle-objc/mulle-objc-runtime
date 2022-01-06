Calls issued by the compiler
=============================

#### [obj ...] regular call

mulle_objc_object_call 
mulle_objc_object_call_inline    
mulle_objc_object_call_inline_partial  

#### [super ...] super call

mulle_objc_object_supercall   
mulle_objc_object_supercall_inline
mulle_objc_object_supercall_inline_partial

#### +[Class ...]  class lookup

mulle_objc_object_lookup_infraclass_nofail   
mulle_objc_object_lookup_infraclass_inline_nofail  
mulle_objc_object_lookup_infraclass_inline_nofail_nofast   
mulle_objc_object_lookup_infraclass_nofast_nofail
mulle_objc_global_lookup_infraclass_nofail
mulle_objc_global_lookup_infraclass_inline_nofail  
mulle_objc_global_lookup_infraclass_nofast_nofail
mulle_objc_global_lookup_infraclass_inline_nofail_nofast   



Calls used by the method cache
===============================

#### initial_cache:

```c
_mulle_objc_object_call_class_needcache
_mulle_objc_object_call_needcache
_mulle_objc_class_superlookup_needcache
_mulle_objc_class_superlookup_needcache
```



#### empty_cache:

```c
_mulle_objc_object_call_class_nocache
_mulle_objc_object_call_slow
_mulle_objc_class_superlookup_implementation_nocache_nofail
_mulle_objc_class_superlookup_implementation_nocache_nofail
```

#### normal_cache:

```c
_mulle_objc_object_call_class_nocache
_mulle_objc_object_call_slow
_mulle_objc_class_superlookup_implementation_nocache_nofail
_mulle_objc_class_superlookup_implementation_nocache_nofail
```


