#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief Tri-state capability status for provider features.
 */
enum class CapabilityStatus {
    Supported,
    NotSupported,
    Unknown,
    Unsupported = NotSupported
};

/**
 * @brief Converts CapabilityStatus to its string representation.
 */
[[nodiscard]] inline constexpr std::string_view to_string(CapabilityStatus status) noexcept {
    switch (status) {
        case CapabilityStatus::Supported:    return "Supported";
        case CapabilityStatus::NotSupported: return "NotSupported";
        case CapabilityStatus::Unknown:      return "Unknown";
    }
    return "Unknown";
}

/**
 * @brief Alias for to_string(CapabilityStatus).
 */
[[nodiscard]] inline constexpr std::string_view capability_status_to_string(CapabilityStatus status) noexcept {
    return to_string(status);
}

/**
 * @brief Parses string to CapabilityStatus.
 */
[[nodiscard]] inline constexpr std::optional<CapabilityStatus> capability_status_from_string(std::string_view str) noexcept {
    if (str == "Supported" || str == "supported" || str == "true" || str == "1") {
        return CapabilityStatus::Supported;
    }
    if (str == "NotSupported" || str == "not_supported" || str == "Unsupported" || str == "unsupported" || str == "false" || str == "0") {
        return CapabilityStatus::NotSupported;
    }
    if (str == "Unknown" || str == "unknown") {
        return CapabilityStatus::Unknown;
    }
    return std::nullopt;
}

/**
 * @brief Snapshot of operational capabilities and health for an AI provider endpoint.
 */
struct ProviderCapabilities {
    using CapabilityStatus = ::CapabilityStatus;

    bool endpoint_reachable{false};
    long latency_ms{-1};
    bool supports_chat_completions{false};
    CapabilityStatus vision_capability{CapabilityStatus::Unknown};
    bool supports_json_mode{false};
    bool supports_model_discovery{false};
    std::vector<std::string> available_models;
    std::string detected_server_flavor;
    std::string raw_status_message;

    // Constructors
    ProviderCapabilities() = default;

    // Inspection Helpers & Predicates
    [[nodiscard]] bool is_functional() const noexcept {
        return endpoint_reachable && supports_chat_completions;
    }

    [[nodiscard]] bool is_healthy() const noexcept {
        return endpoint_reachable && supports_chat_completions;
    }

    [[nodiscard]] bool supports_vision() const noexcept {
        return vision_capability == CapabilityStatus::Supported;
    }

    [[nodiscard]] bool has_model(std::string_view model_name) const noexcept {
        return std::any_of(available_models.begin(), available_models.end(),
                           [model_name](const std::string& m) { return m == model_name; });
    }

    /**
     * @brief Generates a concise human-readable summary for UI status badges and logs.
     */
    [[nodiscard]] std::string summary_text() const {
        if (!endpoint_reachable) {
            if (!raw_status_message.empty()) {
                return "Endpoint unreachable: " + raw_status_message;
            }
            return "Endpoint unreachable";
        }

        std::string summary;
        if (!detected_server_flavor.empty() && detected_server_flavor != "unknown") {
            summary += detected_server_flavor + " | ";
        }

        summary += "Chat: " + std::string(supports_chat_completions ? "OK" : "Unavailable");
        summary += " | Vision: " + std::string(to_string(vision_capability));

        if (latency_ms >= 0) {
            summary += " | " + std::to_string(latency_ms) + "ms";
        }

        if (!available_models.empty()) {
            summary += " | Models: " + std::to_string(available_models.size());
        }

        return summary;
    }

    // C++20 Defaulted Equality Operator
    bool operator==(const ProviderCapabilities& other) const = default;
};
