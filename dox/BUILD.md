# How to build mulle-objc-runtime


## What you get

* `libmulle_objc.a` the mulle-objc-runtime static library along with a
bunch of headers.


## TPS / TRT 

You can compile the runtime in various configurations. You can not build a runtime to support all configurations.  

* **TPS** means: tagged pointers
* **TRT** means: thread local runtime


#### 

 TPS  | TRT | Usage
------|-----|----------------------
  -   | -   | All classes uses same dispatch.
  -   | x   | For plugins that can't use global runtimes.
  x   | -   | Default
  x   | x   | Fast dispatch for special classes
  
  
This is set in the CMakeFile of the project. Define `__MULLE_TPS__` to enable TPS. Define `__MULLE_TRT__` to enable thread local runtime.

The mulle-clang compiler uses `-fobjc-tps` by default. Turn it off with
`-fno-objc-tps`. Use `-fobjc-trt` to enable thread local runtime mode.



### Compatibility

**Runtime** shows the configuration as it is compiled. **Code** has included the runtime's headers and is linked against it.


Runtime | Code   | Description
--------|--------|--------------
Global  | Global | Default
Global  | TRT    | Works, but slower. Mixes with "Global Code" too
TRT     | Global | Crashes
TRT     | TRT    | Works

So loading global code into a TRT enabled runtime is a bad idea. The runtime checks against that.


Runtime | Code   | Description
--------|--------|--------------
No-TPS  | No-TPS | Works
No-TPS  | TPS    | Crashes
TPS     | No-TPS | Works, but slower. Does not mix with "TPS Code"
TPS     | YES    | Works

The runtime does not allow loading of TPS code when not configured for TPS. When configured for TPS, it is possible to load No-TPS code only.

## Prerequisites

#### mulle-aba

[mulle-aba](//github.com/mulle-objc/git/mulle-aba/) provides the
ABA safe freeing of resources.


#### mulle-allocator

[mulle-allocator](//github.com/mulle-objc/git/mulle-allocator/) contains the memory-allocation scheme, that mulle-objc uses.


#### mulle-c11

[mulle-c11](//github.com/mulle-objc/git/mulle-c11/) is a header
that abstracts a small set of non-standardized compiler features.


#### mulle-configuration

[mulle-configuration](//github.com/mulle-objc/git/mulle-configuration/)
are configuration files for building with Xcode or cmake. This is expected to
exist in the project directory root.


#### mulle-concurrent

[mulle-concurrent](//github.com/mulle-objc/git/mulle-concurrent/) contains
lock- and wait-free data structures.


#### mulle-thread

[mulle-thread](//github.com/mulle-objc/git/mulle-thread/) contains
the necessary atomic operations.



## Nice to have

#### mulle-build

[mulle-build](//github.com/mulle-objc/git/mulle-build) is used
to assemble the dependencies together and build the library.

#### mulle-homebrew

[mulle-homebrew](//github.com/mulle-objc/git/mulle-homebrew/) is
support for generating homebrew formulae. This is expected to
exist in `./bin`, if you want to release a fork.

#### mulle-tests

[mulle-tests](//github.com/mulle-objc/git/mulle-tests/) are
scripts to provide an environment for running the tests. This is expected to
exist in `./tests`, if you want to run tests.


### Windows: Installing further prerequisites

Check the [mulle-build README.md](//github.com/mulle-objc/git/mulle-build/README.md)
for instrutions how to get the "Git for Windows" bash going.


### OSX: Install mulle-build using homebrew

Install the [homebrew](//brew.sh/) package manager, then

```
brew tap mulle-kybernetik/software
brew install mulle-build
```

### Linux: Install mulle-build using linuxbrew

Install the [linuxbrew](//linuxbrew.sh/) package manager, then it seems you
may need `python-setuptools` dependency as well:

```
sudo apt-get install python-setuptools
```

and then

```
brew tap mulle-kybernetik/software
brew install mulle-build
```

### All: Install mulle-build using git

```
git clone --branch release https://github.com/mulle-nat/mulle-bootstrap
( cd mulle-bootstrap ; ./install.sh )
git clone --branch release https://github.com/mulle-nat/mulle-build
( cd mulle-build ; ./install.sh )
```

## All: Install mulle-objc-runtime using mulle-build


Grab the latest **mulle-objc-runtime** release and go into the project directory:

```
git clone --branch release https://github.com/mulle-objc/mulle-objc-runtime
cd mulle-objc-runtime
```

Then let **mulle-build** fetch the dependencies and
build **mulle-objc-runtime** in debug mode:

```
mulle-build --debug
```

Build library in release mode and install into `tmp` :

```
mulle-clean ;
mulle-install --prefix /tmp
```

---
## Memo

> Why is it not named `libmulle_objc_runtime.a` ?
> There exists a struct called `mulle_objc_runtime` and this library is part
> of a project that for better or worse is now called **mulle-objc**.
> This unfortunately gets confusing.
> I did not want to prefix the functions with `mulle_objcruntime`, and I did
> not want `mulle_objcruntime_class` which would have been correct. So there...
> Maybe I should have.

