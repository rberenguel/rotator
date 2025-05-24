# Rotator

> A shape rotating game forked from https://github.com/0xf00ff00f

Changes in this fork:

- Installable as a basic PWA on mobile devices or desktop
- Dark mode palete (forced, because I want dark mode always)
- Base layout optimised for iPhone, with 2 columns of 3 shapes
- Harder layout on iPad, with more shapes to choose from (or pass the URL parameter `?wide=true` on a browser)
- Time is now a function of number of shapes (2 minutes by default, 4 on wide/hard)
- Levelling up by adding more segments (from the base 4 to 7) and down automatically
- Increase wobbliness to make it easier to see the whole of the shape… and also more confusing to solve
- Randomised color for each shape to make it more confusing to solve

## Emscripten steps

```
source "~/fromsource/emsdk/emsdk_env.sh"
emsdk activate latest
emcmake cmake -B build -S .
```

----

> [!NOTE]
> The rest of the README is the same as in the original.
> Worth noting I could not figure out the right version of SDL to build on Mac,
> the Emscripten build is fast enough to not be worth it building a local run.

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

This should generate the `game.{
    html, wasm, js}` files.
