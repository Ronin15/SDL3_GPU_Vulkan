#pragma once

#include <memory>
#include <string>

#include <SDL3/SDL_gpu.h>

class Renderer {
public:
    Renderer() = default;
    ~Renderer() noexcept;

    // Delete copy operations
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Delete move operations
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    [[nodiscard]] bool Initialize(SDL_GPUDevice* device, SDL_Window* window);
    void Render(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain);

private:
    // Vertex layout constants
    static inline constexpr size_t POSITION_SIZE = sizeof(float) * 3;
    static inline constexpr size_t COLOR_SIZE = sizeof(float) * 3;
    static inline constexpr size_t VERTEX_STRIDE = POSITION_SIZE + COLOR_SIZE;
    [[nodiscard]] bool LoadShaders();
    [[nodiscard]] bool CreatePipeline();
    [[nodiscard]] bool CreateVertexBuffer();
    [[nodiscard]] SDL_GPUShader* LoadShader(const std::string& filepath, SDL_GPUShaderStage stage);

    void Cleanup() noexcept;

    // RAII wrapper for SDL_GPUBuffer
    struct GPUBufferDeleter {
        SDL_GPUDevice* device;
        void operator()(SDL_GPUBuffer* buffer) const noexcept {
            if (buffer && device) {
                SDL_ReleaseGPUBuffer(device, buffer);
            }
        }
    };

    // RAII wrapper for SDL_GPUTransferBuffer
    struct GPUTransferBufferDeleter {
        SDL_GPUDevice* device;
        void operator()(SDL_GPUTransferBuffer* buffer) const noexcept {
            if (buffer && device) {
                SDL_ReleaseGPUTransferBuffer(device, buffer);
            }
        }
    };

    // RAII wrapper for SDL_GPUShader
    struct GPUShaderDeleter {
        SDL_GPUDevice* device;
        void operator()(SDL_GPUShader* shader) const noexcept {
            if (shader && device) {
                SDL_ReleaseGPUShader(device, shader);
            }
        }
    };

    // RAII wrapper for SDL_GPUGraphicsPipeline
    struct GPUPipelineDeleter {
        SDL_GPUDevice* device;
        void operator()(SDL_GPUGraphicsPipeline* pipeline) const noexcept {
            if (pipeline && device) {
                SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
            }
        }
    };

    // RAII wrapper for SDL_GPUFence
    struct GPUFenceDeleter {
        SDL_GPUDevice* device;
        void operator()(SDL_GPUFence* fence) const noexcept {
            if (fence && device) {
                SDL_ReleaseGPUFence(device, fence);
            }
        }
    };

    SDL_GPUDevice* device_ = nullptr;
    SDL_Window* window_ = nullptr;
    std::unique_ptr<SDL_GPUShader, GPUShaderDeleter> vertex_shader_{nullptr, {nullptr}};
    std::unique_ptr<SDL_GPUShader, GPUShaderDeleter> fragment_shader_{nullptr, {nullptr}};
    std::unique_ptr<SDL_GPUGraphicsPipeline, GPUPipelineDeleter> pipeline_{nullptr, {nullptr}};
    std::unique_ptr<SDL_GPUBuffer, GPUBufferDeleter> vertex_buffer_{nullptr, {nullptr}};
    std::unique_ptr<SDL_GPUTransferBuffer, GPUTransferBufferDeleter> upload_transfer_buffer_{nullptr, {nullptr}};
    std::unique_ptr<SDL_GPUFence, GPUFenceDeleter> upload_fence_{nullptr, {nullptr}};
    bool upload_complete_ = false;
};
