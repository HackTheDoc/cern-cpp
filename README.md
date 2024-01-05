# Cernunnos Lang (CPP Compiler)

Cernunnos is a desktop application programming language.
It aims to be simple and make game dev easier.

This compiler is written in CPP, but a futur one will be self-hosted.

## Building

Requires `nasm` and `ld` on a Linux operating system.

```
$ git clone https://github.com/hackthedoc/cernunnos-cpp
$ cd cernunnos-cpp
mkdir build
cmake -S . -B build
cmake --build build
```

Executable will be `cern` in the `build/` directory.

## Credits

This compiler is greatly inspired from the [hygrogen](https://github.com/orosmatthew/hydrogen-cpp) lang compiler project from *orosmatthew*.

I base it on his videos and add personal modification on it.
