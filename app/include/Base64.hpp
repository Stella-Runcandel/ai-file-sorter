#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief Lightweight, zero-dependency C++20 Base64 encoding and Data URI utility.
 */
class Base64 {
public:
    /**
     * @brief Encode binary data into a standard Base64 string (RFC 4648).
     */
    [[nodiscard]] static std::string encode(std::span<const uint8_t> data) {
        static constexpr char kAlphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        if (data.empty()) {
            return {};
        }

        std::string result;
        result.reserve(((data.size() + 2) / 3) * 4);

        size_t i = 0;
        while (i + 3 <= data.size()) {
            const uint32_t octet_a = data[i++];
            const uint32_t octet_b = data[i++];
            const uint32_t octet_c = data[i++];
            const uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

            result.push_back(kAlphabet[(triple >> 18) & 0x3F]);
            result.push_back(kAlphabet[(triple >> 12) & 0x3F]);
            result.push_back(kAlphabet[(triple >> 6) & 0x3F]);
            result.push_back(kAlphabet[triple & 0x3F]);
        }

        if (i < data.size()) {
            const size_t remaining = data.size() - i;
            const uint32_t octet_a = data[i++];
            const uint32_t octet_b = (remaining > 1) ? data[i++] : 0;
            const uint32_t triple = (octet_a << 16) | (octet_b << 8);

            result.push_back(kAlphabet[(triple >> 18) & 0x3F]);
            result.push_back(kAlphabet[(triple >> 12) & 0x3F]);

            if (remaining == 2) {
                result.push_back(kAlphabet[(triple >> 6) & 0x3F]);
                result.push_back('=');
            } else {
                result.push_back('=');
                result.push_back('=');
            }
        }

        return result;
    }

    /**
     * @brief Convenience overload for std::vector<uint8_t>.
     */
    [[nodiscard]] static std::string encode(const std::vector<uint8_t>& data) {
        return encode(std::span<const uint8_t>(data.data(), data.size()));
    }

    /**
     * @brief Convenience overload for raw pointer and length.
     */
    [[nodiscard]] static std::string encode(const uint8_t* data, size_t length) {
        if (!data || length == 0) return {};
        return encode(std::span<const uint8_t>(data, length));
    }

    /**
     * @brief Constructs a standard data URI: data:<mime_type>;base64,<encoded_data>
     */
    [[nodiscard]] static std::string to_data_uri(std::span<const uint8_t> data,
                                                 std::string_view mime_type = "image/jpeg") {
        std::string uri = "data:";
        uri.append(mime_type);
        uri.append(";base64,");
        uri.append(encode(data));
        return uri;
    }

    [[nodiscard]] static std::string to_data_uri(const std::vector<uint8_t>& data,
                                                 std::string_view mime_type = "image/jpeg") {
        return to_data_uri(std::span<const uint8_t>(data.data(), data.size()), mime_type);
    }
};
