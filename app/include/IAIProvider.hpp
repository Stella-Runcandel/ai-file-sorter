#pragma once

#include "AIRequest.hpp"
#include "AIResponse.hpp"
#include "ProviderCapabilities.hpp"

#include <memory>
#include <string>
#include <string_view>

/**
 * @brief Pure abstract interface for all AI provider implementations.
 *
 * Decouples the application workflow (categorization, renaming, summarization)
 * from specific inference backends, wire protocols, and vendor APIs.
 */
class IAIProvider {
public:
    virtual ~IAIProvider() = default;

    /**
     * @brief Execute a canonical AI inference request (synchronously).
     * @param request Canonical domain request containing model, messages, and parameters.
     * @return Canonical domain response containing result text, status, and metadata.
     */
    [[nodiscard]] virtual AIResponse generate_response(const AIRequest& request) = 0;

    /**
     * @brief Query or probe operational capabilities and health of the provider endpoint.
     * @return Snapshot of provider capabilities (reachability, latency, vision support, model list).
     */
    [[nodiscard]] virtual ProviderCapabilities get_capabilities() = 0;

    /**
     * @brief Human-readable identifier for the provider implementation.
     */
    [[nodiscard]] virtual std::string provider_name() const = 0;
};
