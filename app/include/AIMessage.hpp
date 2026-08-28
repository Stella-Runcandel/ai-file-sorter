#pragma once

#include "AIContentPart.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @brief Author / participant role for an AI conversational message.
 */
enum class Role {
    System,
    User,
    Assistant,
    Tool
};

/**
 * @brief Converts a Role enum to its canonical OpenAI-compatible string representation.
 */
[[nodiscard]] inline constexpr std::string_view role_to_string(Role role) noexcept {
    switch (role) {
        case Role::System:    return "system";
        case Role::User:      return "user";
        case Role::Assistant: return "assistant";
        case Role::Tool:      return "tool";
    }
    return "user";
}

/**
 * @brief Parses a role string into a Role enum.
 */
[[nodiscard]] inline constexpr std::optional<Role> role_from_string(std::string_view str) noexcept {
    if (str == "system")    return Role::System;
    if (str == "user")      return Role::User;
    if (str == "assistant") return Role::Assistant;
    if (str == "tool")      return Role::Tool;
    return std::nullopt;
}

/**
 * @brief Represents a single structured message in an AI conversation.
 *
 * Pure C++20 domain struct supporting multi-part heterogeneous payloads (text and binary images).
 */
struct AIMessage {
    using Role = ::Role;

    Role role{Role::User};
    std::vector<AIContentPart> parts;

    // Static role string conversions scoped to AIMessage
    [[nodiscard]] static constexpr std::string_view role_to_string(Role r) noexcept {
        return ::role_to_string(r);
    }

    [[nodiscard]] static constexpr std::optional<Role> role_from_string(std::string_view str) noexcept {
        return ::role_from_string(str);
    }

    // Constructors
    AIMessage() = default;

    AIMessage(Role r, std::vector<AIContentPart> p)
        : role(r), parts(std::move(p)) {}

    AIMessage(Role r, AIContentPart part)
        : role(r), parts{std::move(part)} {}

    AIMessage(Role r, std::string text)
        : role(r), parts{AIContentPart::from_text(std::move(text))} {}

    // Convenience Factories
    [[nodiscard]] static AIMessage system(std::string text) {
        return AIMessage(Role::System, AIContentPart::from_text(std::move(text)));
    }

    [[nodiscard]] static AIMessage user(std::string text) {
        return AIMessage(Role::User, AIContentPart::from_text(std::move(text)));
    }

    [[nodiscard]] static AIMessage user_multimodal(std::string text,
                                                   std::vector<uint8_t> image_bytes,
                                                   std::string mime_type = "image/jpeg",
                                                   std::optional<std::string> detail = "auto") {
        AIMessage msg;
        msg.role = Role::User;
        if (!text.empty()) {
            msg.parts.push_back(AIContentPart::from_text(std::move(text)));
        }
        msg.parts.push_back(AIContentPart::from_image(std::move(image_bytes),
                                                      std::move(mime_type),
                                                      std::move(detail)));
        return msg;
    }

    [[nodiscard]] static AIMessage assistant(std::string text) {
        return AIMessage(Role::Assistant, AIContentPart::from_text(std::move(text)));
    }

    [[nodiscard]] static AIMessage tool(std::string text) {
        return AIMessage(Role::Tool, AIContentPart::from_text(std::move(text)));
    }

    // Mutation & Builder Helpers
    void add_part(AIContentPart part) {
        parts.push_back(std::move(part));
    }

    void add_text(std::string text) {
        parts.push_back(AIContentPart::from_text(std::move(text)));
    }

    void add_image(std::vector<uint8_t> bytes,
                   std::string mime_type = "image/jpeg",
                   std::optional<std::string> detail = "auto") {
        parts.push_back(AIContentPart::from_image(std::move(bytes),
                                                  std::move(mime_type),
                                                  std::move(detail)));
    }

    // Inspection Helpers
    [[nodiscard]] bool is_multimodal() const noexcept {
        for (const auto& part : parts) {
            if (part.is_image()) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool empty() const noexcept {
        return parts.empty();
    }

    /**
     * @brief Concatenates all text parts into a single string.
     */
    [[nodiscard]] std::string text_content() const {
        std::string result;
        for (const auto& part : parts) {
            if (part.is_text()) {
                result += part.text();
            }
        }
        return result;
    }

    /**
     * @brief Alias for text_content() for extraction adapters.
     */
    [[nodiscard]] std::string extract_text_content() const {
        return text_content();
    }

    // C++20 Defaulted Equality Operator
    bool operator==(const AIMessage& other) const = default;
};
