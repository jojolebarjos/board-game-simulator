# Board game simulator

This C++ header-only library provides a framework for simulating various board games, such as Connect4 and Chinese Checkers.
It is designed for reinforcement learning, where many playthroughs are performed, and relies heavily on template programming and limits head allocations to the minimum.

A unified representation of games is provided through states and actions.
At each state, the current player selects an action from a set of legal moves, leading to a new state.
When a terminal state is reached, a reward is provided, according to the outcome of the playthrough.

For ease of integration with common machine learning frameworks, Python bindings are provided in a [separate repository](https://github.com/jojolebarjos/board-game-simulator-python).


## Getting started

To integrate this library in your project, the recommended way is CMake's FetchContent module.
Add the following to your `CMakeLists.txt` file:

```cmake
include(FetchContent)
FetchContent_Declare(
  game
  GIT_REPOSITORY https://github.com/jojolebarjos/board-game-simulator.git
)
FetchContent_MakeAvailable(game)
```

The [`src`](./src/) and [`tests`](./tests/) folders contain usage examples, which are not included when using FetchContent.
To compile and run them, use CMake directly on this repository:

```
git clone --depth 0 https://github.com/jojolebarjos/board-game-simulator.git
cd board-game-simulator
```

For single-config generators (e.g. MinGW Makefile):

```
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

For multi-config generators (e.g. Visual Studio):

```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_CONFIGURATION_TYPES=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release
```


## References

 * https://github.com/doctest/doctest
 * https://github.com/nlohmann/json
