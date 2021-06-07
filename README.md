# pixel-goo

Goo-like particle system simulation running on the GPU.

Work in progress, hence isn't actually doing the thing it's supposed to yet. The GPU computing works though and it compiles fine (at least on a Mac).

The idea is to make something simmilar to Sebastian Lague's [Ant and Slime Simulations](https://www.youtube.com/watch?v=X-iSQQgOd1A) - particles following one another's trails, but **without** looking at [the code](https://github.com/SebLague/Slime-Simulation).

## Instructions

To build a standalone binary, follow the typical cmake build process:

```
mkdir build
cd build
cmake ..
make
```

and then run `./goo`.

## ToDo's

- [ ] fullscreen mode is a bit janky still
- [ ] no acceleration and velocity shaders
- [ ] no velocity double buffer
- [ ] no frame rendering