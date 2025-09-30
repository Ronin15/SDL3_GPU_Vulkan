# SDL3 GPU Vulkan

A cross-platform graphics application using SDL3's modern GPU API with Vulkan backend.

## Features

- SDL3 GPU API for modern graphics programming
- Vulkan backend for high-performance rendering
- Cross-platform support (Linux, macOS, Windows)
- Automatic shader compilation to SPIR-V
- Modern C++20 codebase

## Requirements

- CMake 3.28 or higher
- Ninja build system
- Vulkan SDK
- C++20 compatible compiler

## Building

### Configure (Debug)
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

### Configure (Release)
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Build
```bash
ninja -C build
```

## Running

### Debug build
```bash
./bin/debug/SDL3_GPU_Vulkan
```

### Release build
```bash
./bin/release/SDL3_GPU_Vulkan
```

## Project Structure

- `src/` - C++ implementation files
- `include/` - C++ header files
- `shaders/` - GLSL shader source files
- `build/` - CMake build directory (generated)
- `bin/` - Binary output directory

## License

MIT License
