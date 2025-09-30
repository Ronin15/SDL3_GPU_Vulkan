#pragma once

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

class Application {
public:
    Application() = default;
    ~Application() noexcept;

    // Delete copy operations
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Delete move operations (resources are non-trivial)
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    [[nodiscard]] bool Initialize();
    void Run();

    // Window dimensions as inline constexpr
    static inline constexpr int WINDOW_WIDTH = 1280;
    static inline constexpr int WINDOW_HEIGHT = 720;

private:
    void Render();
    void Cleanup() noexcept;

    // RAII wrapper for SDL_Window
    struct WindowDeleter {
        void operator()(SDL_Window* window) const noexcept {
            if (window) {
                SDL_DestroyWindow(window);
            }
        }
    };

    // RAII wrapper for SDL_GPUDevice
    struct GPUDeviceDeleter {
        void operator()(SDL_GPUDevice* device) const noexcept {
            if (device) {
                SDL_DestroyGPUDevice(device);
            }
        }
    };

    std::unique_ptr<SDL_Window, WindowDeleter> window_;
    std::unique_ptr<SDL_GPUDevice, GPUDeviceDeleter> device_;
};
