#include "Renderer.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string_view>

#include <SDL3/SDL.h>

Renderer::~Renderer() noexcept {
    Cleanup();
}

bool Renderer::Initialize(SDL_GPUDevice* device, SDL_Window* window) {
    if (!device) {
        std::cerr << "Invalid GPU device provided to Renderer\n";
        return false;
    }

    if (!window) {
        std::cerr << "Invalid window provided to Renderer\n";
        return false;
    }

    device_ = device;
    window_ = window;

    if (!LoadShaders()) {
        Cleanup();
        return false;
    }

    if (!CreatePipeline()) {
        Cleanup();
        return false;
    }

    if (!CreateVertexBuffer()) {
        Cleanup();
        return false;
    }

    std::cout << "Renderer initialized successfully\n";
    return true;
}

SDL_GPUShader* Renderer::LoadShader(const std::string& filepath, SDL_GPUShaderStage stage) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << '\n';
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read shader file: " << filepath << '\n';
        return nullptr;
    }

    SDL_GPUShaderCreateInfo shaderInfo{};
    shaderInfo.code = reinterpret_cast<const Uint8*>(buffer.data());
    shaderInfo.code_size = static_cast<size_t>(size);
    shaderInfo.stage = stage;
    shaderInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    shaderInfo.entrypoint = "main";

    SDL_GPUShader* shader = SDL_CreateGPUShader(device_, &shaderInfo);
    if (!shader) {
        std::cerr << "Failed to create shader from: " << filepath << '\n';
        std::cerr << "SDL Error: " << SDL_GetError() << '\n';
        return nullptr;
    }

    return shader;
}

bool Renderer::LoadShaders() {
    // Get executable directory path
    std::string_view base_path = SDL_GetBasePath();
    if (base_path.empty()) {
        std::cerr << "Failed to get base path: " << SDL_GetError() << '\n';
        return false;
    }

    std::filesystem::path shader_dir = std::filesystem::path(base_path) / "shaders";

    // Construct shader paths relative to executable
    std::string vert_path = (shader_dir / "test.vert.spv").string();
    std::string frag_path = (shader_dir / "test.frag.spv").string();

    SDL_GPUShader* vert = LoadShader(vert_path, SDL_GPU_SHADERSTAGE_VERTEX);
    if (!vert) {
        return false;
    }

    SDL_GPUShader* frag = LoadShader(frag_path, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!frag) {
        SDL_ReleaseGPUShader(device_, vert);
        return false;
    }

    vertex_shader_.reset(vert);
    vertex_shader_.get_deleter().device = device_;

    fragment_shader_.reset(frag);
    fragment_shader_.get_deleter().device = device_;

    std::cout << "Shaders loaded successfully from: " << shader_dir.string() << '\n';
    return true;
}

bool Renderer::CreatePipeline() {
    // Vertex input state - position (vec3) and color (vec3)
    SDL_GPUVertexAttribute vertexAttributes[2]{};
    vertexAttributes[0].location = 0;
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertexAttributes[0].offset = 0;
    vertexAttributes[0].buffer_slot = 0;

    vertexAttributes[1].location = 1;
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertexAttributes[1].offset = POSITION_SIZE;
    vertexAttributes[1].buffer_slot = 0;

    SDL_GPUVertexBufferDescription vertexBufferDesc{};
    vertexBufferDesc.slot = 0;
    vertexBufferDesc.pitch = VERTEX_STRIDE;
    vertexBufferDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexInputState vertexInputState{};
    vertexInputState.vertex_buffer_descriptions = &vertexBufferDesc;
    vertexInputState.num_vertex_buffers = 1;
    vertexInputState.vertex_attributes = vertexAttributes;
    vertexInputState.num_vertex_attributes = 2;

    // Color target state
    SDL_GPUColorTargetDescription colorTargetDesc{};
    colorTargetDesc.format = SDL_GetGPUSwapchainTextureFormat(device_, window_);
    colorTargetDesc.blend_state.enable_blend = false;

    // Graphics pipeline create info
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertex_shader = vertex_shader_.get();
    pipelineInfo.fragment_shader = fragment_shader_.get();
    pipelineInfo.vertex_input_state = vertexInputState;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = &colorTargetDesc;
    pipelineInfo.target_info.has_depth_stencil_target = false;

    // Rasterizer state
    pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pipelineInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    // Multisample state
    pipelineInfo.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device_, &pipelineInfo);
    if (!pipeline) {
        std::cerr << "Failed to create graphics pipeline\n";
        std::cerr << "SDL Error: " << SDL_GetError() << '\n';
        return false;
    }

    pipeline_.reset(pipeline);
    pipeline_.get_deleter().device = device_;

    std::cout << "Graphics pipeline created successfully\n";
    return true;
}

bool Renderer::CreateVertexBuffer() {
    // Triangle vertices: position (x, y, z) and color (r, g, b)
    float vertexData[] = {
        // Position          // Color
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top (red)
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Bottom-left (green)
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // Bottom-right (blue)
    };

    SDL_GPUBufferCreateInfo bufferInfo{};
    bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    bufferInfo.size = sizeof(vertexData);

    SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device_, &bufferInfo);
    if (!buffer) {
        std::cerr << "Failed to create vertex buffer\n";
        std::cerr << "SDL Error: " << SDL_GetError() << '\n';
        return false;
    }

    // Upload vertex data
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = sizeof(vertexData);

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device_, &transferInfo);
    if (!transferBuffer) {
        std::cerr << "Failed to create transfer buffer\n";
        SDL_ReleaseGPUBuffer(device_, buffer);
        return false;
    }

    void* mapped = SDL_MapGPUTransferBuffer(device_, transferBuffer, false);
    if (!mapped) {
        std::cerr << "Failed to map transfer buffer\n";
        SDL_ReleaseGPUTransferBuffer(device_, transferBuffer);
        SDL_ReleaseGPUBuffer(device_, buffer);
        return false;
    }

    SDL_memcpy(mapped, vertexData, sizeof(vertexData));
    SDL_UnmapGPUTransferBuffer(device_, transferBuffer);

    // Copy to GPU buffer
    SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(device_);
    if (!uploadCmd) {
        std::cerr << "Failed to acquire command buffer for upload\n";
        SDL_ReleaseGPUTransferBuffer(device_, transferBuffer);
        SDL_ReleaseGPUBuffer(device_, buffer);
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);

    SDL_GPUTransferBufferLocation srcLocation{};
    srcLocation.transfer_buffer = transferBuffer;
    srcLocation.offset = 0;

    SDL_GPUBufferRegion dstRegion{};
    dstRegion.buffer = buffer;
    dstRegion.offset = 0;
    dstRegion.size = sizeof(vertexData);

    SDL_UploadToGPUBuffer(copyPass, &srcLocation, &dstRegion, false);
    SDL_EndGPUCopyPass(copyPass);

    // Create fence to track upload completion
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(uploadCmd);
    if (!fence) {
        std::cerr << "Failed to create upload fence\n";
        SDL_ReleaseGPUTransferBuffer(device_, transferBuffer);
        SDL_ReleaseGPUBuffer(device_, buffer);
        return false;
    }

    // Store fence and transfer buffer for async tracking
    upload_fence_.reset(fence);
    upload_fence_.get_deleter().device = device_;

    upload_transfer_buffer_.reset(transferBuffer);
    upload_transfer_buffer_.get_deleter().device = device_;

    upload_complete_ = false;

    vertex_buffer_.reset(buffer);
    vertex_buffer_.get_deleter().device = device_;

    std::cout << "Vertex buffer upload submitted (async)\n";
    return true;
}

void Renderer::Render(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain) {
    if (!cmd || !swapchain || !pipeline_ || !vertex_buffer_) {
        return;
    }

    // Check if async upload is complete
    if (!upload_complete_ && upload_fence_) {
        if (SDL_QueryGPUFence(device_, upload_fence_.get())) {
            // Upload finished, release fence and transfer buffer
            upload_fence_.reset();
            upload_transfer_buffer_.reset();  // Safe to release now
            upload_complete_ = true;
            std::cout << "Vertex buffer upload complete\n";
        }
    }

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture = swapchain;
    colorTarget.clear_color = {.r = 0.1f, .g = 0.2f, .b = 0.3f, .a = 1.0f};
    colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, nullptr);

    // Only draw if upload is complete
    if (upload_complete_) {
        SDL_BindGPUGraphicsPipeline(renderPass, pipeline_.get());

        SDL_GPUBufferBinding vertexBinding{};
        vertexBinding.buffer = vertex_buffer_.get();
        vertexBinding.offset = 0;

        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    }

    SDL_EndGPURenderPass(renderPass);
}

void Renderer::Cleanup() noexcept {
    // Clean up in reverse order of creation
    upload_fence_.reset();
    upload_transfer_buffer_.reset();
    vertex_buffer_.reset();
    pipeline_.reset();
    fragment_shader_.reset();
    vertex_shader_.reset();
    upload_complete_ = false;
    window_ = nullptr;
    device_ = nullptr;
}
