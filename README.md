# libingenialink - Motion and communications library for Ingenia servodrives

[![Build Status](https://travis-ci.org/ingeniamc/ingenialink.svg?branch=master)](https://travis-ci.org/ingeniamc/ingenialink)
[![Build status](https://ci.appveyor.com/api/projects/status/wjysv351u0of92xt?svg=true)](https://ci.appveyor.com/project/gmarull/ingenialink)

`libingenialink` is a portable, pure C implementation library for simple motion
control tasks and communications with Ingenia drives.

[![Ingenia Servodrives](https://s3.eu-central-1.amazonaws.com/ingeniamc-cdn/images/all-servodrives.png)](http://www.ingeniamc.com)

## What it can do

The library provides:

* Simple motion control functions (homing, profile position, etc.)
* Communications API for Ingenia drives using the IngeniaLink protocol
  (available on the USB/RS232/RS485 interfaces)
* Register poller and watcher
* Object oriented interface
* Supports single link and daisy-chain topologies
* Device listing and monitor
* Thread-safe communications
* Descriptive and detailed error messages

It is worth to note that the IngeniaLink protocol was not designed for
applications (where high-reliability and bandwidth are likely strong
requirements). You should limit its usage to configuration or evaluation tasks.

## Building libingenialink

The `libingenialink` library is built using [CMake][cmake] (version 3.0 or
newer) on all platforms. It depends on [libsercomm][sercomm], so make sure you
have it installed before building.

On most systems you can build the library using the following commands:

```sh
cmake -H. -B_build
cmake --build _build
```

[cmake]: https://cmake.org
[sercomm]: https://github.com/ingeniamc/sercomm

### Build options

The following build options are available:

- `WITH_EXAMPLES` (OFF): When enabled, the library usage example applications
  will be built.
- `WITH_DOCS` (OFF): When enabled the API documentation can be built.
- `WITH_PIC` (OFF): When enabled, generated code will be position independent.
  This may be useful if you want to embed ingenialink into a dynamic library.

Furthermore, *standard* CMake build options can be used. You may find useful to
read this list of [useful CMake variables][cmakeuseful].

[cmakeuseful]: https://cmake.org/Wiki/CMake_Useful_Variables

## Coding standards

`libingenialink` is written in [ANSI C][ansic] (C99), so any modern compiler
should work. The non-C99 compatible MSVC 9.0 (Visual Studio 2008) is also
supported so that the Python 2.7 extension can be built. However, its support
will likely be dropped in future versions.

Code is written following the [Linux Kernel coding style][kernelstyle]. You can
check for errors or suggestions like this (uses `checkpatch.pl`):

```sh
cmake --build build --target style_check
```

[ansic]: http://en.wikipedia.org/wiki/ANSI_C
[kernelstyle]: https://www.kernel.org/doc/html/latest/process/coding-style.html
