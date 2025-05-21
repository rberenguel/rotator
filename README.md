A shape rotating game forked from https://github.com/0xf00ff00f



## Building

Pull down submodules:

```
$ git submodule update --init --recursive
```

### Native binary

Install dependencies. On Ubuntu:

```
$ sudo apt install libsdl1.2-dev libglew-dev
```

Build with CMake:
    
```
$ cmake -B build -S .
$ cmake --build build --parallel
```

Now run the `game` binary to play.

It should build on MacOSX, though I haven't tested.

### WebAssembly binary

Download and install the Emscripten SDK. Follow the instructions [here](https://emscripten.org/docs/getting_started/downloads.html).

Set up the emsdk environment variables with something like:

```
$ source path/to/emsdk/emsdk_env.sh
```

Build with CMake:

```
$ emcmake cmake -B build -S .
$ cmake --build build --parallel
```

This should generate the `game.{html,wasm,js}` files.
