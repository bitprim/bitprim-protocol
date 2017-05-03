[![Build Status](https://travis-ci.org/bitprim/bitprim-protocol.svg?branch=master)](https://travis-ci.org/bitprim/bitprim-protocol)

# Bitprim Protocol

*Bitcoin Blockchain Query Protocol Library*

## Installation

A build requires boost, zeromq, protobuf, bitprim-libsecp256k1 and bitprim-core. The [bitprim/secp256k1](https://github.com/bitprim/secp256k1) repository is forked from [bitcoin-core/secp256k1](https://github.com/bitcoin-core/secp256k1) in order to control for changes and to incorporate the necessary Visual Studio build. The original repository can be used directly but recent changes to the public interface may cause build breaks. The `--enable-module-recovery` switch is required.

Detailed instructions are provided below.
  * [Debian/Ubuntu](#debianubuntu)
  * [Macintosh](#macintosh)
  * [Windows](#windows)

### Debian/Ubuntu

Bitprim requires a C++11 compiler, currently minimum [GCC 4.8.0](https://gcc.gnu.org/projects/cxx0x.html) or Clang based on [LLVM 3.5](http://llvm.org/releases/3.5.0/docs/ReleaseNotes.html).

To see your GCC version:
```sh
$ g++ --version
```
```
g++ (Ubuntu 4.8.2-19ubuntu1) 4.8.2
Copyright (C) 2013 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
If necessary, upgrade your compiler as follows:
```sh
$ sudo apt-get install g++-4.8
$ sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50
$ sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-4.8 50
```
Next install the [build system](http://wikipedia.org/wiki/GNU_build_system) (Automake minimum 1.14) and git:
```sh
$ sudo apt-get install build-essential autoconf automake libtool pkg-config git
```
Next install the [Boost](http://www.boost.org) (minimum 1.56.0) development package:
```sh
$ sudo apt-get install libboost-all-dev
```
Next install the [ZeroMQ](http://zeromq.org/) (minimum 4.2.0) development package:
```sh
$ git clone https://github.com/zeromq/libzmq
$ cd libzmq
$ ./autogen.sh && ./configure && make -j 4
$ make check
$ sudo make install
$ sudo ldconfig
```
Next install the [Protobuf](https://github.com/google/protobuf) (minimum 3.0.0) development package:
```sh
$ sudo apt-get install autoconf automake libtool curl make g++ unzip
$ git clone https://github.com/google/protobuf
$ cd protobuf
$ ./autogen.sh 
$ ./configure
$ make
$ make check
$ sudo make install
$ sudo ldconfig
```
Next install the [CMake](https://cmake.org/) (minimum 3.7.0-rc1) development package:
```sh
$ wget https://cmake.org/files/v3.7/cmake-3.7.0-rc1.tar.gz
$ tar -xvzf cmake-3.7.0-rc1.tar.gz
$ cd cmake-3.7.0-rc1
$ ./bootstrap
$ make
$ sudo make install
$ sudo ln -s /usr/local/bin/cmake /usr/bin/cmake
```

Next install [bitprim/bitprim-core](https://github.com/bitprim/bitprim-core)

Finally, install bitprim-protocol:
```sh
$ git clone https://github.com/bitprim/bitprim-protocol
$ cd bitprim-protocol
$ mkdir build
$ cd build
$ cmake .. -G "Unix Makefiles" -DWITH_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++11"
$ make
$ sudo make install
$ sudo ldconfig
```
Bitprim-protocol is now installed in `/usr/local/`.

### Macintosh
Bitprim compiles with Clang on OSX and requires C++11 support. Installation has been verified using Clang based on [LLVM 3.5](http://llvm.org/releases/3.5.0/docs/ReleaseNotes.html). This version or newer should be installed as part of the Xcode command line tools.

To see your Clang/LLVM  version:
```sh
$ clang++ --version
```
```
Apple LLVM version 6.0 (clang-600.0.54) (based on LLVM 3.5svn)
Target: x86_64-apple-darwin14.0.0
Thread model: posix
```
If required update your version of the command line tools as follows:
```sh
$ xcode-select --install
```
Install the [build system](http://wikipedia.org/wiki/GNU_build_system) (Automake minimum 1.14) and [wget](http://www.gnu.org/software/wget):
```sh
$ brew install autoconf automake libtool pkgconfig wget
```
Next install the [Boost](http://www.boost.org) (1.56.0 or newer) development package:
```sh
$ brew install boost
```
Next install the [ZeroMQ](http://zeromq.org/) (minimum 4.2.0) development package:
```sh
$ git clone https://github.com/zeromq/libzmq
$ cd libzmq
$ ./autogen.sh && ./configure && make -j 4
$ make check
$ sudo make install
```
Next install the [Protobuf](https://github.com/google/protobuf) (minimum 3.0.0) development package:
```sh
$ brew install protobuf
```
Next install [bitprim/bitprim-core](https://github.com/bitprim/bitprim-core)

Finally, install bitprim-protocol:
```sh
$ git clone https://github.com/bitprim/bitprim-protocol
$ cd bitprim-protocol
$ mkdir build
$ cd build
$ cmake .. -DWITH_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++11"
$ make
$ sudo make install
```
Bitprim-protocol is now installed in `/usr/local/`.

### Windows
```
TODO:Windows Install
```
