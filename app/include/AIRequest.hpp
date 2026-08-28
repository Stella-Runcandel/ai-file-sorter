#pragma once

#include "AIMessage.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Encapsulates a complete canonical AI inference request.
 *
 * Pure C++20 domain struct decoupled from network protocols and JSON serialization formats.
 */
struct AIRequest {
    std::string model;
    std::vector<AIMessage> messages;
    std::optional<float> temperature{std::nullopt};
    std::optional<int> max_tokens{std::nullopt};
    std::optional<std::string> response_format{std::nullopt};
    std::chrono::milliseconds timeout{60000}; // Default 60 seconds

    // Constructors
    AIRequest() = default;

    explicit AIRequest(std::string model_name)
        : model(std::move(model_name)) {}

    AIRequest(std::string model_name, std::vector<AIMessage> msgs)
        : model(std::move(model_name)), messages(std::move(msgs)) {}

    // Builder Helpers (fluent chaining)
    AIRequest& with_model(std::string model_name) {
        model = std::move(model_name);
        return *this;
    }

    AIRequest& with_temperature(float temp) {
        temperature = temp;
        return *this;
    }

    AIRequest& with_max_tokens(int tokens) {
        max_tokens = tokens;
        return *this;
    }

    AIRequest& with_response_format(std::string format) {
        response_format = std::move(format);
        return *this;
    }

    AIRequest& with_timeout(std::chrono::milliseconds to) {
        timeout = to;
        return *this;
    }

    // Message Composition Helpers
    void add_message(AIMessage msg) {
        messages.push_back(std::move(msg));
    }

    void add_system_message(std::string text) {
        messages.push_back(AIMessage::system(std::move(text)));
    }

    void add_user_message(std::string text) {
        messages.push_back(AIMessage::user(std::move(text)));
    }

    void add_user_multimodal_message(std::string text,
                                     std::vector<uint8_t> image_bytes,
                                     std::string mime_type = "image/jpeg",
                                     std::optional<std::string> detail = "auto") {
        messages.push_back(AIMessage::user_multimodal(std::move(text),
                                                      std::move(image_bytes),
                                                      std::move(mime_type),
                                                      std::move(detail)));
    }

    // Inspection Helpers
    [[nodiscard]] bool is_multimodal() const noexcept {
        for (const auto& msg : messages) {
            if (msg.is_multimodal()) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool empty() const noexcept {
        return messages.empty();
    }

    // C++20 Defaulted Equality Operator
    bool operator==(const AIRequest& other) const = default;
};
