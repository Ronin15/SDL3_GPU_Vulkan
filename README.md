# SDL3 GPU Vulkan

A cross-platform graphics application using SDL3's modern GPU API with Vulkan backend.

## Features

- **Modern GPU rendering** with SDL3's cross-platform GPU API and Vulkan backend
- **Production-ready architecture** with async resource loading and proper resource management
- **Clean, maintainable code** following C++20 best practices and RAII principles
- **Cross-platform** support for Linux, macOS, and Windows
- **Automated build system** with shader compilation and portable runtime paths
- **Example rendering** with a colored triangle demonstrating the complete graphics pipeline

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
