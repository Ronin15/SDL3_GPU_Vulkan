# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an SDL3 GPU + Vulkan rendering project that uses SDL3's modern GPU API with Vulkan backend for cross-platform graphics. The project leverages SDL3's abstraction layer over Vulkan to simplify GPU programming while maintaining high performance.

## Build System

The project uses CMake 3.28+ with Ninja as the build generator. FetchContent automatically downloads and builds SDL3 from source.

### Common Build Commands

**Initial configuration (Debug):**
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

**Initial configuration (Release):**
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

**Build the project:**
```bash
ninja -C build
```

**Reconfigure build type:**
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release  # or Debug
ninja -C build
```

**Clean build:**
```bash
rm -rf build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja -C build
```

### Binary Output Locations

- Debug builds: `bin/debug/SDL3_GPU_Vulkan`
- Release builds: `bin/release/SDL3_GPU_Vulkan`

### Running the Application

```bash
# Debug build
./bin/debug/SDL3_GPU_Vulkan

# Release build
./bin/release/SDL3_GPU_Vulkan
```

## Project Structure

- `src/` - C++ implementation files (*.cpp)
- `include/` - C++ header files (*.hpp)
- `shaders/` - GLSL shader files (*.vert, *.frag, *.comp, etc.)
- `build/` - CMake build directory (generated)
- `bin/debug/` - Debug binaries
- `bin/release/` - Release binaries

## Coding Standards

This project strictly follows **C++20 best practices**. All code must adhere to these standards.

### C++20 Language Features

**Required modern C++ features:**
- Use `#pragma once` instead of include guards
- Use `[[nodiscard]]` for functions returning important values (e.g., `Initialize()`)
- Use `[[maybe_unused]]` for intentionally unused parameters
- Use `noexcept` on destructors and functions that cannot throw
- Use `= default` in headers for trivial constructors/destructors
- Use `inline constexpr` for compile-time constants in headers
- Use aggregate initialization with `{}` for zero-initialization
- Use designated initializers where appropriate (with C compatibility considerations)
- Use `EXIT_SUCCESS`/`EXIT_FAILURE` instead of magic numbers (0, 1)

### Resource Management

**No raw pointers for resource ownership.** Use RAII principles:
- Use `std::unique_ptr` with custom deleters for C API resources
- Custom deleters must be `noexcept`
- Ensure proper cleanup order in destructors
- Resources must be exception-safe
- Delete copy/move operations for non-trivial resource classes

### Code Organization

**Header files (*.hpp):**
- Use `#pragma once` for include guards
- Include order: C++ standard library → third-party → project headers
- Define constants as `static inline constexpr` class members
- Declare all special member functions explicitly (rule of five/zero)
- Mark member variables with trailing underscore (e.g., `window_`)

**Implementation files (*.cpp):**
- Include own header first
- Include order: own header → C++ stdlib → third-party → project headers
- Use anonymous namespaces for internal linkage (file-local functions/constants)
- Use `'\n'` instead of `std::endl` (avoid unnecessary flush)

### Naming Conventions

- Classes: PascalCase (e.g., `Application`)
- Functions/methods: PascalCase (e.g., `Initialize()`)
- Member variables: snake_case with trailing underscore (e.g., `window_`)
- Constants: SCREAMING_SNAKE_CASE or inline constexpr (e.g., `WINDOW_WIDTH`)
- Local variables: snake_case (e.g., `raw_window`)

## Architecture

### SDL3 GPU Integration

The project uses SDL3's GPU API, which provides a modern, low-level graphics abstraction. Key components:

1. **GPU Device** - Created with `SDL_CreateGPUDevice()` using SPIRV shader format and Vulkan backend
2. **Window Management** - Standard SDL3 window claimed for GPU rendering via `SDL_ClaimWindowForGPUDevice()`
3. **Command Buffers** - Acquired per-frame with `SDL_AcquireGPUCommandBuffer()`
4. **Render Passes** - Created with `SDL_BeginGPURenderPass()` for rendering operations
5. **Swapchain** - Managed automatically by SDL3, accessed via `SDL_AcquireGPUSwapchainTexture()`

### Application Class

The main application is defined in `include/Application.hpp` and implemented in `src/Application.cpp`:
- `Initialize()` - Sets up SDL3, GPU device, and window using RAII wrappers
- `Run()` - Main event loop handling input and rendering
- `Render()` - Per-frame rendering using GPU command buffers and render passes
- Uses `std::unique_ptr` with custom deleters for SDL resources (no raw pointers)

### Shader Compilation

Shaders are automatically compiled during the build process:
- Source: `shaders/` directory (*.vert, *.frag, *.comp, *.geom, *.tesc, *.tese)
- Output: `build/shaders/*.spv` (SPIR-V bytecode)
- Compiler: `glslc` (part of Vulkan SDK, found via CMake)

## Platform-Specific Notes

### Debug vs Release Builds

- **Debug**:
  - Console output enabled (SDL_MAIN_HANDLED)
  - Full debug symbols (-g3)
  - No optimization (-O0)
  - macOS: dSYM bundles generated automatically

- **Release**:
  - Aggressive optimization (-O3, LTO, vectorization)
  - Platform-specific tuning (Apple Silicon: -mcpu=apple-m1, x86-64: AVX2/FMA)
  - Windows: GUI subsystem (no console)
  - macOS: Application bundle

### macOS Development

- Debug builds include LLDB-optimized debug info (-glldb, -fstandalone-debug)
- dSYM bundles generated at `bin/debug/SDL3_GPU_Vulkan.dSYM`
- Debug symbols preserved (no stripping)

### Linux Development

- Requires Vulkan SDK and development libraries
- Uses x86-64-v3 optimizations in release builds (SSE4.2, AVX, AVX2, FMA)

## Dependencies

- **SDL3** - Fetched automatically from GitHub (main branch)
- **Vulkan SDK** - Must be installed system-wide (required for both headers and glslc)
- **CMake 3.28+** - Required for build configuration

## Key Configuration Options

Defined in `CMakeLists.txt:112-116`:
- `SDL_SHARED=TRUE` - Build SDL3 as shared library
- `SDL_STATIC=FALSE` - Don't build static library
- `SDL_TEST=FALSE` - Exclude SDL test framework
- `SDL_GPU=ON` - Enable SDL_GPU API
- `SDL_GPU_VULKAN=ON` - Enable Vulkan backend for SDL_GPU
