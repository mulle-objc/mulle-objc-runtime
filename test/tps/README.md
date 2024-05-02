## How to build 32 bit on 64 bit

[How to Compile 32-bit Apps on 64-bit Ubuntu?](//stackoverflow.com/questions/22355436/how-to-compile-32-bit-apps-on-64-bit-ubuntu)

For gcc the `-m32` flag is the ticket.

```
mulle-sde -v -DCFLAGS=-m32 test craft
mulle-sde -v -DMULLE_ARCH=i686 -DCFLAGS=-m32 test run
```

## Prerequisites

```
sudo apt-get install gcc-multilib libc6-dev:i386
```