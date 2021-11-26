# pixel-goo

Goo-like particle system simulation running on the GPU.

Work in progress, hence isn't actually doing the thing it's supposed to yet. The GPU computing works though and it compiles fine (at least on a Mac).

The idea is to make something similar to Sebastian Lague's [Ant and Slime Simulations](https://www.youtube.com/watch?v=X-iSQQgOd1A) - particles following one another's trails, but **without** looking at [the code](https://github.com/SebLague/Slime-Simulation).

## Instructions

To build a standalone binary, follow the typical cmake build process:

```bash
mkdir build
cd build
cmake ..
make
```

and then run `./goo`.


## Cross-compile on macos for windows

To compile for windows on macos first [install mingw32 toolchain](https://blog.filippo.io/easy-windows-and-linux-cross-compilers-for-macos/):

```bash
brew install FiloSottile/musl-cross/musl-cross
brew install mingw-w64
```

then follow a similar process, this time specifying the target as Windows and compiler as the `x86_64-w64-mingw32-` toolchain.

```bash
mkdir build-windows
cd build-windows
cmake \
    -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
    -DCMAKE_C_FLAGS="-static" -DCMAKE_CXX_FLAGS="-static" -DCMAKE_EXE_LINKER_FLAGS="-static" \
    -DCMAKE_C_COMPILER_WORKS=1 -DCMAKE_CXX_COMPILER_WORKS=1 \
    ..
make
```

Note that system-dependent libraries have to be statically linked (`-static`), and that we have to prevent cmake from checking  whether the compiler works (`...COMPILER_WORKS=1`) since it teh test program will also be cross-compiled and therefore not work on a mac.


## ToDo's

- [ ] add text rendering shader
- [x] automagically include shader files at compile time
- [ ] fullscreen mode is still a bit janky
- [x] no acceleration and velocity shaders
- [x] no velocity double buffer
- [x] fix segfault buggs
- [ ] ? add frame rendering
- [ ] trail buffer colormap sampling
- [ ] screen rendering shader could blend between density and trail colormap
- [ ] ? better lerp in screen rendering shader
