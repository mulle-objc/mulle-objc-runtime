These are compiler tests for mulle-clang. They are in Objective-C.

```
mulle-test --dir-name test-compiler --project-dialect objc
```
@0_compiler

These tests run within the MulleObjC tests, but they can run even if
MulleObjC is not present. They do not depend on MulleObjC headers. Just the
runtime.

The runtime does not include these tests as it is C only.

