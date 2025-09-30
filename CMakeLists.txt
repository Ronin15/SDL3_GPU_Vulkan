cmake_minimum_required(VERSION 3.28)
project(SDL3_GPU_Vulkan VERSION 1.0.0 LANGUAGES C CXX)

# Set build configurations
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Choose the type of build (Debug or Release)" FORCE)
endif()

# Define available build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Set output directory paths based on build type
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/bin")
set(OUTPUT_DEBUG_DIR "${OUTPUT_BASE_DIR}/debug")
set(OUTPUT_RELEASE_DIR "${OUTPUT_BASE_DIR}/release")

# Configure output directories based on the current build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DEBUG_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DEBUG_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DEBUG_DIR})
else() # Release build
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_RELEASE_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_RELEASE_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_RELEASE_DIR})
endif()

# Debug build settings
if(APPLE)
    # macOS-specific debug flags with dSYM generation and LLDB compatibility
    set(CMAKE_C_FLAGS_DEBUG 
        "-g3 -O0 -gdwarf-4 -glldb -fno-limit-debug-info -DDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG 
        "-g3 -O0 -Wall -Wextra -gdwarf-4 -glldb -fno-limit-debug-info -fstandalone-debug -DDEBUG")
    
    # Ensure debug symbols are preserved (don't strip them)
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -g")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")
else()
    # Other platforms
    set(CMAKE_C_FLAGS_DEBUG 
        "${CMAKE_C_FLAGS_DEBUG} -g3 -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -Wall -Wextra -DDEBUG")
endif()

# Release build settings
if(APPLE)
    # macOS-specific release flags
    set(CMAKE_C_FLAGS_RELEASE 
        "-O3 -flto -fno-strict-aliasing -fomit-frame-pointer \
        -ffunction-sections -fdata-sections -ffast-math -ftree-vectorize \
        -fpeel-loops -funroll-loops")
    set(CMAKE_CXX_FLAGS_RELEASE 
        "-O3 -flto -fno-strict-aliasing -fomit-frame-pointer \
        -ffunction-sections -fdata-sections -ffast-math -ftree-vectorize \
        -fpeel-loops -funroll-loops")
    
    # Add Apple Silicon specific optimizations if on Apple Silicon
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mcpu=apple-m1")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mcpu=apple-m1")
    endif()
else()
    # x86-64 specific flags for other platforms
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -flto \
        -fno-strict-aliasing -fomit-frame-pointer -ffunction-sections \
        -fdata-sections -Wl,--gc-sections -ffast-math -ftree-vectorize \
        -fpeel-loops -funroll-loops -march=x86-64-v3 -mtune=generic \
        -msse4.2 -mavx -mavx2 -mfma")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto \
        -fno-strict-aliasing -fomit-frame-pointer -ffunction-sections \
        -fdata-sections -Wl,--gc-sections -ffast-math -ftree-vectorize \
        -fpeel-loops -funroll-loops -march=x86-64-v3 -mtune=generic \
        -msse4.2 -mavx -mavx2 -mfma")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for use with tools like clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Copy compile_commands.json to project root after build
add_custom_target(copy-compile-commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)

# Include FetchContent module
include(FetchContent)

# Avoid network updates during configure; use existing cached deps
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Find Vulkan (required for SDL_GPU Vulkan backend)
find_package(Vulkan REQUIRED)

# Specify the stable release tag for SDL3
set(SDL3_GIT_TAG "main")  # Use main to get latest SDL_GPU features

# Download and configure SDL3 with GPU support
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG ${SDL3_GIT_TAG}
)

# Build options for SDL3 with GPU enabled
set(SDL_SHARED TRUE CACHE BOOL "Build a SDL shared library")
set(SDL_STATIC FALSE CACHE BOOL "Build a SDL static library")
set(SDL_TEST FALSE CACHE BOOL "Build the SDL test framework")
set(SDL_GPU ON CACHE BOOL "Enable SDL_GPU" FORCE)
set(SDL_GPU_VULKAN ON CACHE BOOL "Enable Vulkan backend for SDL_GPU" FORCE)

# Make SDL3 available
FetchContent_MakeAvailable(SDL3)

# Find all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Specify include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    SDL3::SDL3
    Vulkan::Vulkan
)

# Shader compilation setup
find_program(GLSLC glslc HINTS ${Vulkan_GLSLC_EXECUTABLE} REQUIRED)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

# Find all shader files
file(GLOB_RECURSE SHADER_SOURCES
    ${SHADER_SOURCE_DIR}/*.vert
    ${SHADER_SOURCE_DIR}/*.frag
    ${SHADER_SOURCE_DIR}/*.comp
    ${SHADER_SOURCE_DIR}/*.geom
    ${SHADER_SOURCE_DIR}/*.tesc
    ${SHADER_SOURCE_DIR}/*.tese
)

# Compile each shader
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)
    
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
    )
    
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

# Add custom target for shaders
if(SHADER_OUTPUTS)
    add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})
    add_dependencies(${PROJECT_NAME} shaders)
endif()

# Platform-specific settings
if(UNIX AND NOT APPLE)
    # For Linux targets
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Enable SDL console output for Linux debug builds
        target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)
        message(STATUS "SDL console output enabled for debug build")
    endif()
endif()

if(WIN32)
    # For Windows targets
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Enable SDL console output for Windows debug builds
        target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

        # Set the subsystem to console for debug builds
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE FALSE
        )

        message(STATUS "SDL console output enabled for debug build")
    else()
        # Use Windows subsystem for Release builds (no console)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()

    # Copy SDL3.dll to build directory on Windows
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

if(APPLE)
    # For macOS targets
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Enable SDL console output for macOS debug builds
        target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

        # On macOS, applications are bundles by default, but we can ensure
        # that console output works properly in debug mode
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE FALSE      # Don't create a bundle in debug mode
            BUILD_WITH_INSTALL_RPATH TRUE
            BUILD_RPATH "@executable_path"
            INSTALL_RPATH "@executable_path"
            # Enable dSYM generation for debug symbols
            XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
            XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS YES
        )

        # Generate dSYM bundle for debugging with verbose output
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND dsymutil $<TARGET_FILE:${PROJECT_NAME}> 
                -o $<TARGET_FILE:${PROJECT_NAME}>.dSYM
            COMMAND echo "dSYM bundle created at: $<TARGET_FILE:${PROJECT_NAME}>.dSYM"
            COMMENT "Generating dSYM bundle for debugging"
        )
        
        # Prevent accidental stripping of debug symbols
        set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT NO
            XCODE_ATTRIBUTE_COPY_PHASE_STRIP NO
            XCODE_ATTRIBUTE_STRIP_STYLE "non-global"
        )

        message(STATUS "SDL console output enabled for macOS debug build")
        message(STATUS "dSYM generation enabled for debug symbols")
        message(STATUS "LLDB debugging flags enabled (-glldb, -fstandalone-debug)")
    else()
        # For release builds, create a proper macOS application bundle
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            BUILD_WITH_INSTALL_RPATH TRUE
            BUILD_RPATH "@executable_path/../Frameworks;@executable_path"
            INSTALL_RPATH "@executable_path/../Frameworks;@executable_path"
        )
    endif()
endif()

# Create output directories if they don't exist
file(MAKE_DIRECTORY ${OUTPUT_DEBUG_DIR})
file(MAKE_DIRECTORY ${OUTPUT_RELEASE_DIR})

# Display status message indicating successful configuration
message(STATUS "SDL3 with GPU support and Vulkan backend configured successfully")
message(STATUS "Debug binaries will be output to: ${OUTPUT_DEBUG_DIR}")
message(STATUS "Release binaries will be output to: ${OUTPUT_RELEASE_DIR}")
