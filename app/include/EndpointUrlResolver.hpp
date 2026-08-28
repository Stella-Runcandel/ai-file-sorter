#pragma once

#include <string>
#include <string_view>

/**
 * @brief Centralized, robust URL normalizer and router for OpenAI-compatible endpoints.
 *
 * Resolves standard vendor URLs (Ollama, LM Studio, llama-server, LocalAI, vLLM, OpenAI,
 * OpenRouter) into compliant /v1/chat/completions and /v1/models endpoints without
 * routing bugs (such as omitting /v1 for Ollama).
 */
class EndpointUrlResolver {
public:
    static constexpr std::string_view kDefaultOpenAIChatEndpoint =
        "https://api.openai.com/v1/chat/completions";
    static constexpr std::string_view kDefaultOpenAIModelsEndpoint =
        "https://api.openai.com/v1/models";

    /**
     * @brief Normalizes any user-supplied base URL or full endpoint into the canonical
     *        /chat/completions endpoint URL.
     *
     * Rules:
     * - Empty input -> https://api.openai.com/v1/chat/completions
     * - "http://localhost:11434" -> "http://localhost:11434/v1/chat/completions" (Ollama fix)
     * - "http://localhost:11434/v1" -> "http://localhost:11434/v1/chat/completions"
     * - "http://localhost:11434/v1/chat/completions" -> "http://localhost:11434/v1/chat/completions"
     * - "http://localhost:1234" -> "http://localhost:1234/v1/chat/completions" (LM Studio)
     * - "http://localhost:1234/v1" -> "http://localhost:1234/v1/chat/completions"
     * - "http://localhost:8080" -> "http://localhost:8080/v1/chat/completions" (llama-server)
     * - "https://api.openai.com" -> "https://api.openai.com/v1/chat/completions"
     */
    [[nodiscard]] static std::string resolve_chat_completions_url(std::string_view input_url);

    /**
     * @brief Normalizes any user-supplied base URL into the canonical /models discovery endpoint.
     *
     * Rules:
     * - Empty input -> https://api.openai.com/v1/models
     * - "http://localhost:11434" -> "http://localhost:11434/v1/models"
     * - "http://localhost:11434/v1" -> "http://localhost:11434/v1/models"
     * - "http://localhost:11434/v1/chat/completions" -> "http://localhost:11434/v1/models"
     * - "http://localhost:11434/chat/completions" -> "http://localhost:11434/models"
     */
    [[nodiscard]] static std::string resolve_models_url(std::string_view input_url);

    /**
     * @brief Extracts host and port (or domain) without paths for concise logging.
     *        e.g. "http://localhost:11434/v1/chat/completions" -> "localhost:11434"
     */
    [[nodiscard]] static std::string extract_host_display(std::string_view input_url);

    /**
     * @brief Strips user:password credentials and query parameters for safe logging.
     */
    [[nodiscard]] static std::string sanitize_url_for_logging(std::string_view input_url);

private:
    [[nodiscard]] static std::string trim_ws_and_slashes(std::string_view input);
    [[nodiscard]] static bool ends_with_ci(std::string_view str, std::string_view suffix);
};
