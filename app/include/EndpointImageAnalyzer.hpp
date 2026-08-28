#pragma once

#include "ImageAnalyzer.hpp"
#include "IAIProvider.hpp"

#include <memory>
#include <filesystem>

/**
 * @brief Multimodal image analyzer adapter for endpoint-based vision analysis,
 * implementing the ImageAnalyzer interface via IAIProvider.
 *
 * Implements strict AI boundary isolation:
 * - Decodes and preprocesses images entirely in-memory (max 2048x2048).
 * - Encodes to Base64 Data URI.
 * - Constructs canonical AIRequest with AIContentPart.
 * - Dispatches multimodal request to user-provided OpenAI-compatible endpoints.
 * - Zero local model execution, zero GGUF dependencies, zero silent fallback.
 */
class EndpointImageAnalyzer : public ImageAnalyzer {
public:
    explicit EndpointImageAnalyzer(std::shared_ptr<IAIProvider> provider);
    ~EndpointImageAnalyzer() override = default;

    ImageAnalysisResult analyze(const std::filesystem::path& image_path) override;

private:
    std::shared_ptr<IAIProvider> provider_;
};
