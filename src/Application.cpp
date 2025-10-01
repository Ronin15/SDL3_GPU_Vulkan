#include "Application.hpp"

#include <iostream>

Application::~Application() noexcept {
    Cleanup();
}

bool Application::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
        return false;
    }

    // Create window first
    SDL_Window* raw_window = SDL_CreateWindow(
        "SDL3 GPU + Vulkan",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!raw_window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << '\n';
        return false;
    }

    // Create GPU device with Vulkan backend
    SDL_GPUDevice* raw_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV,
        true,  // debug mode
        nullptr
    );

    if (!raw_device) {
        SDL_DestroyWindow(raw_window);
        std::cerr << "Failed to create GPU device: " << SDL_GetError() << '\n';
        return false;
    }

    // Claim window for GPU rendering
    if (!SDL_ClaimWindowForGPUDevice(raw_device, raw_window)) {
        SDL_DestroyGPUDevice(raw_device);
        SDL_DestroyWindow(raw_window);
        std::cerr << "Failed to claim window: " << SDL_GetError() << '\n';
        return false;
    }

    // Transfer ownership to smart pointers
    window_.reset(raw_window);
    device_.reset(raw_device);

    // Initialize renderer
    renderer_ = std::make_unique<Renderer>();
    if (!renderer_->Initialize(device_.get(), window_.get())) {
        std::cerr << "Failed to initialize renderer\n";
        return false;
    }

    std::cout << "SDL3 GPU initialized successfully with Vulkan backend\n";
    return true;
}

void Application::Run() {
    bool running = true;
    SDL_Event event{};

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        Render();
    }
}

void Application::Render() {
    if (!device_ || !window_ || !renderer_) {
        return;
    }

    // Acquire command buffer
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device_.get());
    if (!cmd) {
        return;
    }

    // Acquire swapchain texture
    SDL_GPUTexture* swapchain = nullptr;
    if (!SDL_AcquireGPUSwapchainTexture(cmd, window_.get(), &swapchain, nullptr, nullptr)) {
        return;
    }

    if (swapchain) {
        renderer_->Render(cmd, swapchain);
    }

    // Submit command buffer
    SDL_SubmitGPUCommandBuffer(cmd);
}

void Application::Cleanup() noexcept {
    // Clean up renderer first (releases GPU resources while device is still valid)
    renderer_.reset();

    // Release GPU device before window (proper cleanup order)
    if (device_ && window_) {
        SDL_ReleaseWindowFromGPUDevice(device_.get(), window_.get());
    }

    // Smart pointers handle cleanup automatically in correct order
    device_.reset();
    window_.reset();
    SDL_Quit();
}
