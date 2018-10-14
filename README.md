# mulle-objc-runtime

A portable Objective-C runtime written in C11. No Assembler required.
It follows the Apple "Objective-C 1 Runtime" and
[adds many features](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_present_and_absent.html)
from "Objective-C 2.0", but the function calls are completely different.
It is designed to be suitable for massive multi-threading.

Fork      |  Build Status | Release Version
----------|---------------|-----------------------------------
[Mulle kybernetiK](https://github.com/mulle-objc/mulle-objc/tree/release) | [![Build Status](https://travis-ci.org/mulle-objc/mulle-objc.svg)](https://travis-ci.org/mulle-objc/mulle-objc) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-objc/mulle-objc.svg) [![Build Status](https://travis-ci.org/mulle-objc/mulle-objc.svg?branch=release)](https://travis-ci.org/mulle-objc/mulle-objc)


## What's so different ?

* Runtime objects (like classes and selectors) are not referenced by name, but
by a unique ID. No special linker action required.

* Parameters outside of `self` and `_cmd` are passed via `_param`, a single
pointer to a struct. This simplifies a lot of code, especially forwarding code.
Return values are also returned with the same struct. Optimizations are done for
simple methods with only none or one parameter and none or one return value.

* It uses inlineable method calls for superior performance. The user can
specify "fast" classes and "fast" methods for extra speed in performance
critical cases.

* `retain`/`release` semantics are built in. These are non-overridable, which
makes them a lot faster.

* `isa` is not part of the instance, but instead prefixed to the instance.

* `Protocol` as a type and an object does not exist anymore. Instead there is
PROTOCOL which is basically the same as SEL and has compiler support.

* No global lock, except when loading code. The runtime in normal operation
only locks during `+initialize` on a per class basis.

* Protections against the fragile base class problem

* Multiple runtimes can coexist in differently named "universes".

## Required Libraries and Tools

![Libraries and Tools](https://raw.githubusercontent.com/mulle-objc/mulle-objc-runtime/release/dox/mulle-objc-runtime-dependencies.png)

  Name         | Build Status | Release Version
---------------|--------------|---------------------------------
[mulle-aba](//github.com/mulle-c/mulle-aba) | [![Build Status](https://travis-ci.org/mulle-c/mulle-aba.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-aba) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-aba.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-aba.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-aba)
[mulle-allocator](//github.com/mulle-c/mulle-allocator) | [![Build Status](https://travis-ci.org/mulle-c/mulle-allocator.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-allocator) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-allocator.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-allocator.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-allocator)
[mulle-c11](//github.com/mulle-c/mulle-c11) | [![Build Status](https://travis-ci.org/mulle-c/mulle-c11.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-c11) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-c11.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-c11.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-c11)
[mulle-concurrent](//github.com/mulle-c/mulle-concurrent) | [![Build Status](https://travis-ci.org/mulle-c/mulle-concurrent.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-concurrent) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-concurrent.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-concurrent.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-concurrent)
[mulle-thread](//github.com/mulle-c/mulle-thread) | [![Build Status](https://travis-ci.org/mulle-c/mulle-thread.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-thread) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-thread.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-thread.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-thread)
[mulle-vararg](//github.com/mulle-c/mulle-vararg) | [![Build Status](https://travis-ci.org/mulle-c/mulle-vararg.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-vararg) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-vararg.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-vararg.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-vararg)

## Install

See [mulle-objc-developer](//github.com/mulle-objc/mulle-objc-developer) for
installation instructions.


## How to build it

#### Manually with cmake

Install all above prerequisites:

now build the project

```
mkdir build
cd build
cmake ..
```

#### Conveniently with mulle-sde

Install [mulle-sde]/(//github.com/mulle-sde) and run `mulle-sde craft`.


## How to use it

If you haven't used an Objective-C runtime before, it is useful to get to know
the much better documented "Mac OS X Objective-C 1.0" runtime first.
[Intro to the Objective-C Runtime](//mikeash.com/pyblog/friday-qa-2009-03-13-intro-to-the-objective-c-runtime.html)
could be a good starting point.


> #### C Caveat
>
> It you use `.c` files that include `<mulle-objc-runtime/mulle-objc-runtime.h>`
> make sure that you compile with `__MULLE_OBJC_TPS__`, `__MULLE_OBJC_NO_TPS__`
> `__MULLE_OBJC_FCS__`  `__MULLE_OBJC_NO_FCS__` as defined when compiling the
> runtime. Since C-only compilations do not emit runtime information,
> mismatches can not be checked by the runtime.
> Easy fix: rename `.c` to `.m` and use **mulle-clang**


### Data structures

API                                                  | Description
-----------------------------------------------------|-----------------------------------
[`_mulle_objc_class`](dox/API_CLASS.md)              | Deal with Classes
[`_mulle_objc_ivar`](dox/API_IVAR.md) et al.         | Instance variables
[`_mulle_objc_loadinfo`](dox/API_LOADINFO.md)        | Install Classes, Categories, Methods, Strings into the runtime
[`_mulle_objc_method`](dox/API_METHOD.md)  et al.    | Deal with Methods
[`_mulle_objc_object`](dox/API_OBJECT.md)  et al.    | Deal with Instances
[`_mulle_objc_property`](dox/API_PROPERTY.md) et al. | Handle Properties
[`_mulle_objc_runtime`](dox/API_RUNTIME.md)          | Work with the runtime


### Other functions

API                                   | Description
--------------------------------------|-----------------------------------
[Global Functions](dox/API_GLOBAL.md) | Global functions  and conveniences
[Exceptions](dox/API_EXCEPTION.md)    | Raising exceptions
[Vararg extensions](dox/API_VARARG.md)| Dealing with variable arguments


## Articles

These articles give you some background about the **mulle-objc** runtime:

1. [mulle-objc: a new Objective-C runtime](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_a_new_objective_c_.html)
2. [mulle-objc: a meta calling convention](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_meta_call_convention.html)
3. [mulle-objc: removing superflous ifs](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_the_superflous_if.html)
3. [mulle-objc: inlined messaging](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_inlined_messaging.html)
4. [mulle-objc: some research about selectors](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_selector_statistics.html)
5. [mulle-objc: hashes for classes, selectors and protocols](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_selectors_are_hashes.html)
6. [mulle_objc: object layout, retain counting, finalize](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_finalize_makes_a_comeback.html)
7. [mulle_objc: inheriting methods from protocols](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_inheriting_from_protocols.html)
8. [mulle_objc: present and absent language features](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_present_and_absent.html)
9. [mulle_objc: the trouble with @property](//www.mulle-kybernetik.com/weblog/2016/mulle_objc_property_trouble.html)
10. [mulle_objc: ivar layout with @property](//www.mulle-kybernetik.com/weblog/2016/mulle_objc_ivar_layout.html)
11. mulle_objc: technically speaking
   1. [mulle-clang, technically speaking](//www.mulle-kybernetik.com/weblog/2016/mulle_objc_clang_technically.html)
   2. [mulle-objc-runtime, technically speaking](//www.mulle-kybernetik.com/weblog/2016/mulle_objc_runtime_technically.html)
   3. [MulleFoundation, technically speaking](//www.mulle-kybernetik.com/weblog/2016/mulle_objc_foundation_technically.html)

If something is unclear, feel free to contact the author.


### Platforms and Compilers

All platforms and compilers supported by
[mulle-c11](//github.com/mulle-nat/mulle-c11/) and
[mulle-thread](//github.com/mulle-nat/mulle-thread/).


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)

