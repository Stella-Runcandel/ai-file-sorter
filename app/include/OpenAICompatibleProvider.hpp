#pragma once

#include "Base64.hpp"
#include "EndpointUrlResolver.hpp"
#include "IAIProvider.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Universal HTTP REST provider for any OpenAI-compatible API endpoint.
 *
 * Communicates with local (Ollama, LM Studio, llama-server, LocalAI, vLLM) and
 * remote (OpenAI, OpenRouter, Groq, Mistral, self-hosted) endpoints via standard
 * /v1/chat/completions and /v1/models JSON protocols.
 *
 * Implements client-side base64 data URI construction for multimodal images,
 * model discovery, health probing, and credential masking.
 */
class OpenAICompatibleProvider : public IAIProvider {
public:
    /**
     * @brief Operational configuration for the provider.
     */
    struct Config {
        std::string endpoint_url{"https://api.openai.com/v1"};
        std::string api_key;
        std::string default_model;
        std::chrono::milliseconds timeout{60000};
        bool log_requests{false};

        bool operator==(const Config& other) const = default;
    };

    /**
     * @brief Custom HTTP transport hook for unit testing and mock injection.
     */
    struct HttpResponse {
        long status_code{0};
        std::string body;
        std::string error_message;
        long latency_ms{0};
    };

    using HttpTransportFn = std::function<HttpResponse(const std::string& url,
                                                       const std::vector<std::string>& headers,
                                                       const std::string& payload,
                                                       std::chrono::milliseconds timeout)>;

    // Constructors
    explicit OpenAICompatibleProvider(Config config);
    OpenAICompatibleProvider(std::string endpoint_url,
                             std::string api_key,
                             std::string default_model);
    ~OpenAICompatibleProvider() override = default;

    // IAIProvider Interface Implementation
    [[nodiscard]] AIResponse generate_response(const AIRequest& request) override;
    [[nodiscard]] ProviderCapabilities get_capabilities() override;
    [[nodiscard]] std::string provider_name() const override { return "OpenAICompatible"; }

    // Configuration Accessors
    [[nodiscard]] const Config& config() const noexcept { return m_config; }
    void update_config(Config config) { m_config = std::move(config); }

    // Direct Discovery & Probing
    [[nodiscard]] std::vector<std::string> fetch_available_models();
    [[nodiscard]] ProviderCapabilities probe_endpoint();

    // Testing / Dependency Injection
    void set_http_transport_for_testing(HttpTransportFn transport) {
        m_custom_transport = std::move(transport);
    }

    // Public Wire-Format Serialization & Deserialization (Exposed for Testing)
    [[nodiscard]] static std::string serialize_chat_request(const AIRequest& request,
                                                            const std::string& fallback_model);
    [[nodiscard]] static AIResponse deserialize_chat_response(const std::string& json_response,
                                                              long http_status,
                                                              long latency_ms);
    [[nodiscard]] static std::vector<std::string> parse_models_list(const std::string& json_response);
    [[nodiscard]] static std::string mask_api_key(const std::string& key);

private:
    Config m_config;
    HttpTransportFn m_custom_transport;

    [[nodiscard]] HttpResponse execute_http_post(const std::string& url,
                                                 const std::string& payload,
                                                 std::chrono::milliseconds timeout) const;
    [[nodiscard]] HttpResponse execute_http_get(const std::string& url,
                                                std::chrono::milliseconds timeout) const;

    [[nodiscard]] std::vector<std::string> build_headers() const;
    [[nodiscard]] static HttpResponse perform_curl_request(const std::string& url,
                                                           const std::vector<std::string>& headers,
                                                           const std::string& payload,
                                                           bool is_post,
                                                           std::chrono::milliseconds timeout);
};
