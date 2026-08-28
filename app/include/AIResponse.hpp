#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

/**
 * @brief Token usage statistics returned by the AI provider.
 */
struct Usage {
    int prompt_tokens{0};
    int completion_tokens{0};
    int total_tokens{0};

    [[nodiscard]] constexpr bool is_zero() const noexcept {
        return prompt_tokens == 0 && completion_tokens == 0 && total_tokens == 0;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return is_zero();
    }

    /**
     * @brief Computes effective total tokens (falls back to prompt + completion if total is 0).
     */
    [[nodiscard]] constexpr int effective_total_tokens() const noexcept {
        return (total_tokens > 0) ? total_tokens : (prompt_tokens + completion_tokens);
    }

    bool operator==(const Usage& other) const = default;
};

// Compatibility type alias
using AIUsage = Usage;

/**
 * @brief Encapsulates a complete canonical AI inference response.
 *
 * Fully decoupled from transport protocol (HTTP/REST) and JSON serialization formats.
 */
struct AIResponse {
    using Usage = ::Usage;

    bool success{false};
    std::string content;
    std::string model;
    Usage usage{};
    std::string finish_reason{"stop"};
    long http_status{0};
    double latency_ms{0.0};
    std::optional<std::string> error_message{std::nullopt};

    // Constructors
    AIResponse() = default;

    // Factory Helpers
    [[nodiscard]] static AIResponse make_success(std::string response_content,
                                                 std::string model_name = "",
                                                 Usage token_usage = {},
                                                 std::string finish_reason_str = "stop",
                                                 long status_code = 200,
                                                 double elapsed_latency_ms = 0.0) {
        AIResponse resp;
        resp.success = true;
        resp.content = std::move(response_content);
        resp.model = std::move(model_name);
        resp.usage = token_usage;
        resp.finish_reason = std::move(finish_reason_str);
        resp.http_status = status_code;
        resp.latency_ms = elapsed_latency_ms;
        resp.error_message = std::nullopt;
        return resp;
    }

    [[nodiscard]] static AIResponse make_failure(std::string error_msg,
                                                 long status_code = 0,
                                                 double elapsed_latency_ms = 0.0,
                                                 std::string model_name = "") {
        AIResponse resp;
        resp.success = false;
        resp.content.clear();
        resp.model = std::move(model_name);
        resp.usage = Usage{};
        resp.finish_reason.clear();
        resp.http_status = status_code;
        resp.latency_ms = elapsed_latency_ms;
        resp.error_message = std::move(error_msg);
        return resp;
    }

    // Convenience alias matching make_failure
    [[nodiscard]] static AIResponse make_error(std::string error_msg,
                                               long status_code = 0,
                                               double elapsed_latency_ms = 0.0,
                                               std::string model_name = "") {
        return make_failure(std::move(error_msg), status_code, elapsed_latency_ms, std::move(model_name));
    }

    // Accessors & Predicates
    [[nodiscard]] bool is_success() const noexcept {
        return success;
    }

    [[nodiscard]] bool has_error() const noexcept {
        return !success || error_message.has_value();
    }

    [[nodiscard]] const std::string& text() const noexcept {
        return content;
    }

    [[nodiscard]] std::string error_text_or(std::string_view fallback = "Unknown error") const {
        if (error_message.has_value() && !error_message->empty()) {
            return *error_message;
        }
        return std::string(fallback);
    }

    // HTTP Status Classification Helpers
    [[nodiscard]] bool is_rate_limited() const noexcept {
        return http_status == 429;
    }

    [[nodiscard]] bool is_auth_error() const noexcept {
        return http_status == 401 || http_status == 403;
    }

    [[nodiscard]] bool is_server_error() const noexcept {
        return http_status >= 500 && http_status <= 599;
    }

    // C++20 Defaulted Equality Operator
    bool operator==(const AIResponse& other) const = default;
};
