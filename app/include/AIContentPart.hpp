#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

/**
 * @brief Represents a single discrete part of an AI message (text or binary image).
 *
 * Pure C++20 domain value type with zero external dependencies (no Qt, no JSON).
 * Uses std::variant for memory-efficient and exception-safe discriminated union semantics.
 */
class AIContentPart {
public:
    /**
     * @brief Content part discriminator.
     */
    enum class Kind {
        Text,
        Image
    };

    /**
     * @brief Raw binary image payload and metadata for multimodal requests.
     */
    struct ImageData {
        std::vector<uint8_t> bytes;
        std::string mime_type{"image/jpeg"};
        std::optional<std::string> detail{"auto"};

        [[nodiscard]] bool empty() const noexcept { return bytes.empty(); }
        [[nodiscard]] size_t size() const noexcept { return bytes.size(); }
        bool operator==(const ImageData& other) const = default;
    };

    // Constructors
    AIContentPart() : m_data(std::string{}) {}

    explicit AIContentPart(std::string text)
        : m_data(std::move(text)) {}

    explicit AIContentPart(ImageData image)
        : m_data(std::move(image)) {}

    // Factory methods
    [[nodiscard]] static AIContentPart from_text(std::string text) {
        return AIContentPart(std::move(text));
    }

    [[nodiscard]] static AIContentPart from_text(const char* text) {
        return AIContentPart(std::string(text ? text : ""));
    }

    [[nodiscard]] static AIContentPart from_text(std::string_view text) {
        return AIContentPart(std::string(text));
    }

    [[nodiscard]] static AIContentPart from_image(std::vector<uint8_t> bytes,
                                                  std::string mime_type = "image/jpeg",
                                                  std::optional<std::string> detail = "auto") {
        ImageData img;
        img.bytes = std::move(bytes);
        img.mime_type = std::move(mime_type);
        img.detail = std::move(detail);
        return AIContentPart(std::move(img));
    }

    [[nodiscard]] static AIContentPart from_image(ImageData image) {
        return AIContentPart(std::move(image));
    }

    // Const Accessors
    [[nodiscard]] Kind kind() const noexcept {
        return std::holds_alternative<std::string>(m_data) ? Kind::Text : Kind::Image;
    }

    [[nodiscard]] bool is_text() const noexcept {
        return std::holds_alternative<std::string>(m_data);
    }

    [[nodiscard]] bool is_image() const noexcept {
        return std::holds_alternative<ImageData>(m_data);
    }

    const std::string& text() const {
        return std::get<std::string>(m_data);
    }

    const ImageData& image() const {
        return std::get<ImageData>(m_data);
    }

    // Mutable Accessors & Mutators
    std::string& text() {
        return std::get<std::string>(m_data);
    }

    ImageData& image() {
        return std::get<ImageData>(m_data);
    }

    void set_text(std::string text) {
        m_data = std::move(text);
    }

    void set_image(ImageData image) {
        m_data = std::move(image);
    }

    // C++20 Defaulted Equality Operator
    bool operator==(const AIContentPart& other) const = default;

private:
    std::variant<std::string, ImageData> m_data;
};
