[![CMake](https://github.com/cracked-machine/cpp_tlc5955/actions/workflows/cmake.yml/badge.svg)](https://github.com/cracked-machine/cpp_tlc5955/actions/workflows/cmake.yml)
[![Codecov](https://img.shields.io/codecov/c/github/cracked-machine/cpp_tlc5955)](https://app.codecov.io/gh/cracked-machine/cpp_tlc5955)
## TLC5955 48 Channel RGB LED driver library


See the [wiki](https://github.com/cracked-machine/cpp_tlc5955/wiki) for documentation / reference

See the [readme](tests) for test info.

#### Adding this library to your STM32 Project

There are two ways to add this library to your project's CMakeLists.txt:

1. Implicitly include the [external.cmake](cmake/external.cmake):

```
set(BUILD_NAME "MyProject")
add_executable(${BUILD_NAME} "")
include(cmake/external.cmake)
```

2. Explicitly add the [embedded_utils](https://github.com/cracked-machine/embedded_utils.git) and [stm32_interrupt_managers](https://github.com/cracked-machine/stm32_interrupt_managers.git) to your project as submodules and add the subdirectories:

```
add_subdirectory(extern/embedded_utils)
add_subdirectory(extern/stm32_interrupt_managers)
```

