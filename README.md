# libingenialink - Motion and communications library for Ingenia servodrives

[![Build Status](https://travis-ci.org/ingeniamc/ingenialink.svg?branch=master)](https://travis-ci.org/ingeniamc/ingenialink)
[![Build status](https://ci.appveyor.com/api/projects/status/wjysv351u0of92xt?svg=true)](https://ci.appveyor.com/project/gmarull/ingenialink)

`libingenialink` is a portable, pure C implementation library for simple motion
control tasks and communications with Ingenia drives.

[![Ingenia Servodrives](https://s3.eu-central-1.amazonaws.com/ingeniamc-cdn/images/all-servodrives.png)](http://www.ingeniamc.com)

## What it can do

The library provides:

* Simple motion control functions (homing, profile position, etc.)
* Communications API for Ingenia drives (multiple protocols supported)
* Load and use IngeniaDictionary XML dictionaries
* Operate directly using units (e.g. degrees, rad/s, etc.)
* Register polling and monitoring for scope applications
* Servo listing and monitoring
* Object oriented interface
* Thread-safe communications
* Descriptive and detailed error messages

## Building libingenialink

`libingenialink` depends on [libsercomm][sercomm] and [libxml2][libxml2]. A
couple of sections below you will find some instructions on how to build and
install them. `libingenialink` can be built and installed on any system like
this:

```sh
cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=$INSTALL
cmake --build _build
cmake --build _build --target install
```

Note that a `INSTALL` is the installation folder.

[sercomm]: https://github.com/ingeniamc/sercomm
[libxml2]: https://xmlsoft.org

### Build options

The following build options are available:

- `WITH_PROT_EUSB` (ON): Build `EUSB` protocol support.
- `WITH_PROT_MCB` (OFF): Build `MCB` protocol support (EXPERIMENTAL).
- `WITH_EXAMPLES` (OFF): When enabled, the library usage example applications
  will be built.
- `WITH_DOCS` (OFF): When enabled the API documentation can be built.
- `WITH_PIC` (OFF): When enabled, generated code will be position independent.
  This may be useful if you want to embed ingenialink into a dynamic library.

Furthermore, *standard* CMake build options can be used. You may find useful to
read this list of [useful CMake variables][cmakeuseful].

[cmakeuseful]: https://cmake.org/Wiki/CMake_Useful_Variables

## Dependencies

As mentioned before, `libingenialink` depends on [libsercomm][sercomm] and
[libxml2][libxml2], both referenced in the [external][external] folder as
submodules. Therefore, if building them make sure to initialize the submodules
first:

```sh
git submodule update --init --recursive
```

Below you can find some building instructions for dependencies. Note that
`INSTALL` is the installation folder.

[sercomm]: https://github.com/ingeniamc/sercomm
[libxml2]: https://xmlsoft.org
[external]: https://github.com/ingeniamc/ingenialink/tree/master/external

### libsercomm

`libsercomm` also uses CMake, so it can be built and installed on any system
like this:

```sh
cd external/sercomm
cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=$INSTALL
cmake --build _build --target install
```

### libxml2

Athough `libxml2` is multiplatform, the building process can be somewhat painful
on some systems, specially on Windows. This is why we provide a CMake script
to build it on the systems we support. It can be built and installed like this:

```sh
cd external/libxml2
cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=$INSTALL
cmake --build _build --target install
```

If using Linux, we actually recommend installing the library packages from
the official repositories. For example in Debian/Ubuntu systems:

```sh
sudo apt install libxml2-dev
```

On recent versions of macOS, it seems to be already installed on the system. If
not, you can also use [brew][brew] to install it.

[brew]: https://brew.sh

## Coding standards

`libingenialink` is written in [ANSI C][ansic] (C99), so any modern compiler
should work.

Code is written following the [Linux Kernel coding style][kernelstyle]. You can
check for errors or suggestions like this (uses `checkpatch.pl`):

```sh
cmake --build build --target style_check
```

[ansic]: http://en.wikipedia.org/wiki/ANSI_C
[kernelstyle]: https://www.kernel.org/doc/html/latest/process/coding-style.html
