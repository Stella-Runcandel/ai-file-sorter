/**
 * @file test_ai_domain_models.cpp
 * @brief Catch2 v3 unit tests for Canonical AI Domain Models (Milestone 1 / R1).
 * Covers AIContentPart, AIMessage, AIRequest, AIResponse, and ProviderCapabilities.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "AIContentPart.hpp"
#include "AIMessage.hpp"
#include "AIRequest.hpp"
#include "AIResponse.hpp"
#include "ProviderCapabilities.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ============================================================================
// 1. AIContentPart Unit Tests
// ============================================================================

TEST_CASE("AIContentPart - Text part creation and access", "[tier1][domain][content_part]") {
    SECTION("Basic text part instantiation") {
        const std::string text = "Organize tax documents into Fiscal/2026";
        auto part = AIContentPart::from_text(text);

        REQUIRE(part.kind() == AIContentPart::Kind::Text);
        REQUIRE(part.is_text());
        REQUIRE_FALSE(part.is_image());
        REQUIRE(part.text() == text);
    }

    SECTION("Text part instantiation from string_view") {
        std::string_view sv = "Classify legal contracts";
        auto part = AIContentPart::from_text(sv);

        REQUIRE(part.kind() == AIContentPart::Kind::Text);
        REQUIRE(part.is_text());
        REQUIRE(part.text() == "Classify legal contracts");
    }

    SECTION("Empty text string handling") {
        auto empty_part = AIContentPart::from_text("");

        REQUIRE(empty_part.kind() == AIContentPart::Kind::Text);
        REQUIRE(empty_part.is_text());
        REQUIRE(empty_part.text().empty());
    }

    SECTION("Unicode, multi-line, and special character support") {
        const std::string utf8_text = "Taxonomy: 财务/发票/2026 🚀 \n Special: <tag>&\"' \t\r\n";
        auto part = AIContentPart::from_text(utf8_text);

        REQUIRE(part.text() == utf8_text);
    }

    SECTION("Text part with embedded null bytes") {
        const std::string null_text("prefix\0suffix", 13);
        auto part = AIContentPart::from_text(null_text);

        REQUIRE(part.text().size() == 13);
        REQUIRE(part.text() == null_text);
    }

    SECTION("Mutators and in-place updates") {
        AIContentPart part;
        REQUIRE(part.is_text());
        REQUIRE(part.text().empty());

        part.set_text("Updated text content");
        REQUIRE(part.is_text());
        REQUIRE(part.text() == "Updated text content");

        part.text() += " (appended)";
        REQUIRE(part.text() == "Updated text content (appended)");

        AIContentPart::ImageData img;
        img.bytes = {0xAA, 0xBB};
        part.set_image(img);
        REQUIRE(part.is_image());
        REQUIRE_FALSE(part.is_text());
        REQUIRE(part.image().bytes.size() == 2);
    }
}

TEST_CASE("AIContentPart - Image part creation, MIME types, and detail parameters", "[tier1][domain][content_part]") {
    const std::vector<uint8_t> jpeg_header = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46};

    SECTION("Image part with default MIME type and default detail") {
        auto part = AIContentPart::from_image(jpeg_header);

        REQUIRE(part.kind() == AIContentPart::Kind::Image);
        REQUIRE_FALSE(part.is_text());
        REQUIRE(part.is_image());
        REQUIRE(part.image().bytes == jpeg_header);
        REQUIRE(part.image().mime_type == "image/jpeg");
        REQUIRE(part.image().detail.has_value());
        REQUIRE(part.image().detail.value() == "auto");
        REQUIRE_FALSE(part.image().empty());
        REQUIRE(part.image().size() == jpeg_header.size());
    }

    SECTION("Image part with explicit PNG MIME type and high detail") {
        const std::vector<uint8_t> png_header = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        auto part = AIContentPart::from_image(png_header, "image/png", "high");

        REQUIRE(part.image().bytes == png_header);
        REQUIRE(part.image().mime_type == "image/png");
        REQUIRE(part.image().detail.has_value());
        REQUIRE(part.image().detail.value() == "high");
    }

    SECTION("Image part with low detail and omitted detail") {
        const std::vector<uint8_t> webp_header = {0x52, 0x49, 0x46, 0x46};

        auto part_low = AIContentPart::from_image(webp_header, "image/webp", "low");
        REQUIRE(part_low.image().detail.value() == "low");

        auto part_none = AIContentPart::from_image(webp_header, "image/webp", std::nullopt);
        REQUIRE_FALSE(part_none.image().detail.has_value());
    }

    SECTION("Empty image buffer creation") {
        std::vector<uint8_t> empty_bytes;
        auto part = AIContentPart::from_image(empty_bytes, "image/jpeg", "auto");

        REQUIRE(part.is_image());
        REQUIRE(part.image().bytes.empty());
        REQUIRE(part.image().empty());
        REQUIRE(part.image().size() == 0);
    }

    SECTION("Mutable access to image data") {
        auto part = AIContentPart::from_image({0x01, 0x02});
        part.image().bytes.push_back(0x03);
        part.image().mime_type = "image/png";

        REQUIRE(part.image().bytes.size() == 3);
        REQUIRE(part.image().mime_type == "image/png");
    }
}

TEST_CASE("AIContentPart - Exception safety on variant access and boundary conditions", "[tier2][domain][content_part]") {
    SECTION("Calling text() on Image part throws std::bad_variant_access") {
        auto img_part = AIContentPart::from_image({0x01, 0x02, 0x03});
        CHECK_THROWS_AS(img_part.text(), std::bad_variant_access);

        const auto const_img = img_part;
        CHECK_THROWS_AS(const_img.text(), std::bad_variant_access);
    }

    SECTION("Calling image() on Text part throws std::bad_variant_access") {
        auto txt_part = AIContentPart::from_text("standard text content");
        CHECK_THROWS_AS(txt_part.image(), std::bad_variant_access);

        const auto const_txt = txt_part;
        CHECK_THROWS_AS(const_txt.image(), std::bad_variant_access);
    }

    SECTION("Const correctness on accessors") {
        const auto const_text_part = AIContentPart::from_text("const text");
        REQUIRE(const_text_part.text() == "const text");

        const auto const_img_part = AIContentPart::from_image({0xAA, 0xBB});
        REQUIRE(const_img_part.image().bytes.size() == 2);
    }
}

TEST_CASE("AIContentPart - Equality comparisons, copy, and move semantics", "[tier1][tier2][domain][content_part]") {
    SECTION("Equality comparisons across identical and distinct parts") {
        auto t1 = AIContentPart::from_text("Receipt 2026");
        auto t2 = AIContentPart::from_text("Receipt 2026");
        auto t3 = AIContentPart::from_text("Invoice 2026");
        auto i1 = AIContentPart::from_image({0x01, 0x02}, "image/png", "auto");
        auto i2 = AIContentPart::from_image({0x01, 0x02}, "image/png", "auto");
        auto i3 = AIContentPart::from_image({0x01, 0x03}, "image/png", "auto");
        auto i4 = AIContentPart::from_image({0x01, 0x02}, "image/jpeg", "auto");

        REQUIRE(t1 == t2);
        REQUIRE_FALSE(t1 == t3);
        REQUIRE_FALSE(t1 == i1);
        REQUIRE(i1 == i2);
        REQUIRE_FALSE(i1 == i3);
        REQUIRE_FALSE(i1 == i4);
    }

    SECTION("Copy and move semantics on large binary payloads") {
        std::vector<uint8_t> large_buffer(2 * 1024 * 1024, 0xEE); // 2 MB buffer
        auto original = AIContentPart::from_image(large_buffer, "image/jpeg", "high");

        // Copy test
        AIContentPart copied = original;
        REQUIRE(copied == original);
        REQUIRE(copied.image().bytes.size() == 2 * 1024 * 1024);

        // Move test
        AIContentPart moved = std::move(original);
        REQUIRE(moved.image().bytes.size() == 2 * 1024 * 1024);
        REQUIRE(moved.image().bytes[0] == 0xEE);
    }
}

// ============================================================================
// 2. AIMessage Unit Tests
// ============================================================================

TEST_CASE("AIMessage - Factory helpers and role mapping", "[tier1][domain][message]") {
    SECTION("System message helper") {
        auto msg = AIMessage::system("You are a deterministic file classifier.");
        REQUIRE(msg.role == AIMessage::Role::System);
        REQUIRE(msg.parts.size() == 1);
        REQUIRE(msg.parts[0].is_text());
        REQUIRE(msg.parts[0].text() == "You are a deterministic file classifier.");
        REQUIRE(AIMessage::role_to_string(AIMessage::Role::System) == "system");
        REQUIRE(role_to_string(Role::System) == "system");
    }

    SECTION("User text-only message helper") {
        auto msg = AIMessage::user("Classify: budget_2026.xlsx");
        REQUIRE(msg.role == AIMessage::Role::User);
        REQUIRE(msg.parts.size() == 1);
        REQUIRE(msg.parts[0].is_text());
        REQUIRE(msg.parts[0].text() == "Classify: budget_2026.xlsx");
        REQUIRE(AIMessage::role_to_string(AIMessage::Role::User) == "user");
        REQUIRE(role_to_string(Role::User) == "user");
    }

    SECTION("Assistant message helper") {
        auto msg = AIMessage::assistant("Finance/Budgets/2026");
        REQUIRE(msg.role == AIMessage::Role::Assistant);
        REQUIRE(msg.parts.size() == 1);
        REQUIRE(msg.parts[0].is_text());
        REQUIRE(msg.parts[0].text() == "Finance/Budgets/2026");
        REQUIRE(AIMessage::role_to_string(AIMessage::Role::Assistant) == "assistant");
        REQUIRE(role_to_string(Role::Assistant) == "assistant");
    }

    SECTION("Tool role string mapping and parsing") {
        auto msg = AIMessage::tool("tool execution output");
        REQUIRE(msg.role == Role::Tool);
        REQUIRE(AIMessage::role_to_string(AIMessage::Role::Tool) == "tool");
        REQUIRE(role_to_string(Role::Tool) == "tool");

        REQUIRE(role_from_string("system") == Role::System);
        REQUIRE(role_from_string("user") == Role::User);
        REQUIRE(role_from_string("assistant") == Role::Assistant);
        REQUIRE(role_from_string("tool") == Role::Tool);
        REQUIRE_FALSE(role_from_string("invalid_role").has_value());
        REQUIRE(AIMessage::role_from_string("user") == Role::User);
    }

    SECTION("User multimodal message helper") {
        const std::vector<uint8_t> scan_bytes = {0xFF, 0xD8, 0xFF, 0xEE};
        auto msg = AIMessage::user_multimodal("Analyze invoice scan", scan_bytes, "image/jpeg", "high");

        REQUIRE(msg.role == AIMessage::Role::User);
        REQUIRE(msg.parts.size() == 2);
        REQUIRE(msg.parts[0].is_text());
        REQUIRE(msg.parts[0].text() == "Analyze invoice scan");
        REQUIRE(msg.parts[1].is_image());
        REQUIRE(msg.parts[1].image().bytes == scan_bytes);
        REQUIRE(msg.parts[1].image().mime_type == "image/jpeg");
        REQUIRE(msg.parts[1].image().detail == "high");
        REQUIRE(msg.is_multimodal());
    }

    SECTION("User multimodal message with empty text") {
        const std::vector<uint8_t> scan_bytes = {0x01, 0x02};
        auto msg = AIMessage::user_multimodal("", scan_bytes);

        REQUIRE(msg.role == Role::User);
        REQUIRE(msg.parts.size() == 1);
        REQUIRE(msg.parts[0].is_image());
        REQUIRE(msg.is_multimodal());
    }
}

TEST_CASE("AIMessage - Parts vector traversal, multi-content, and extraction helpers", "[tier2][domain][message]") {
    SECTION("Interleaved text and multiple image parts traversal") {
        AIMessage msg;
        msg.role = AIMessage::Role::User;
        msg.add_text("Document Page 1:");
        msg.add_image({0x01}, "image/jpeg");
        msg.add_text("Document Page 2:");
        msg.add_image({0x02}, "image/png");

        REQUIRE(msg.parts.size() == 4);
        REQUIRE(msg.is_multimodal());

        size_t text_count = 0;
        size_t image_count = 0;
        for (const auto& part : msg.parts) {
            if (part.is_text()) ++text_count;
            if (part.is_image()) ++image_count;
        }
        REQUIRE(text_count == 2);
        REQUIRE(image_count == 2);
    }

    SECTION("Empty message parts and multimodal query") {
        AIMessage empty_msg;
        REQUIRE(empty_msg.empty());
        REQUIRE(empty_msg.parts.empty());
        REQUIRE_FALSE(empty_msg.is_multimodal());

        auto text_msg = AIMessage::user("Plain text message");
        REQUIRE_FALSE(text_msg.empty());
        REQUIRE_FALSE(text_msg.is_multimodal());
    }

    SECTION("Text content aggregation helper") {
        AIMessage msg;
        msg.parts.push_back(AIContentPart::from_text("Hello"));
        msg.parts.push_back(AIContentPart::from_image({0x00}, "image/jpeg"));
        msg.parts.push_back(AIContentPart::from_text(" World"));

        REQUIRE(msg.text_content() == "Hello World");
        REQUIRE(msg.extract_text_content() == "Hello World");
    }

    SECTION("Equality and copy semantics") {
        auto msg1 = AIMessage::user("Test prompt");
        auto msg2 = AIMessage::user("Test prompt");
        auto msg3 = AIMessage::assistant("Test prompt");

        REQUIRE(msg1 == msg2);
        REQUIRE_FALSE(msg1 == msg3);

        AIMessage copy = msg1;
        REQUIRE(copy == msg1);
    }
}

// ============================================================================
// 3. AIRequest Unit Tests
// ============================================================================

TEST_CASE("AIRequest - Default initialization and optional parameter settings", "[tier1][domain][request]") {
    SECTION("Default request parameter state") {
        AIRequest req;
        REQUIRE(req.model.empty());
        REQUIRE(req.messages.empty());
        REQUIRE(req.empty());
        REQUIRE_FALSE(req.temperature.has_value());
        REQUIRE_FALSE(req.max_tokens.has_value());
        REQUIRE_FALSE(req.response_format.has_value());
        REQUIRE(req.timeout == std::chrono::milliseconds(60000)); // Default 60s
    }

    SECTION("Custom optional parameter assignment via fluent chaining") {
        AIRequest req("gpt-4o");
        req.with_temperature(0.2f)
           .with_max_tokens(4096)
           .with_response_format("{\"type\": \"json_object\"}")
           .with_timeout(std::chrono::milliseconds(30000));

        REQUIRE(req.model == "gpt-4o");
        REQUIRE(req.temperature.has_value());
        REQUIRE(req.temperature.value() == 0.2f);
        REQUIRE(req.max_tokens.has_value());
        REQUIRE(req.max_tokens.value() == 4096);
        REQUIRE(req.response_format.has_value());
        REQUIRE(req.response_format.value() == "{\"type\": \"json_object\"}");
        REQUIRE(req.timeout == std::chrono::milliseconds(30000));
    }
}

TEST_CASE("AIRequest - Multi-message conversation assembly and multimodal detection", "[tier1][tier2][domain][request]") {
    SECTION("Multi-turn conversation flow assembly") {
        AIRequest req;
        req.with_model("qwen-2.5-coder");
        req.add_system_message("System instruction");
        req.add_user_message("Categorize 1");
        req.add_message(AIMessage::assistant("Category 1"));
        req.add_user_message("Categorize 2");

        REQUIRE(req.messages.size() == 4);
        REQUIRE_FALSE(req.empty());
        REQUIRE(req.messages[0].role == AIMessage::Role::System);
        REQUIRE(req.messages[1].role == AIMessage::Role::User);
        REQUIRE(req.messages[2].role == AIMessage::Role::Assistant);
        REQUIRE(req.messages[3].role == AIMessage::Role::User);
        REQUIRE_FALSE(req.is_multimodal());
    }

    SECTION("Multimodal auto-detection when any message contains image parts") {
        AIRequest text_req;
        text_req.add_system_message("System");
        text_req.add_user_message("Text");
        REQUIRE_FALSE(text_req.is_multimodal());

        AIRequest vision_req;
        vision_req.add_system_message("System");
        vision_req.add_user_multimodal_message("Scan", {0x01, 0x02}, "image/png");
        REQUIRE(vision_req.is_multimodal());
    }

    SECTION("Boundary values for temperature and token limits") {
        AIRequest req;
        req.temperature = 0.0f; // Deterministic
        REQUIRE(req.temperature.value() == 0.0f);
        req.temperature = 2.0f; // Maximum temperature
        REQUIRE(req.temperature.value() == 2.0f);

        req.max_tokens = 1;
        REQUIRE(req.max_tokens.value() == 1);
        req.max_tokens = 128000;
        REQUIRE(req.max_tokens.value() == 128000);
    }

    SECTION("Equality comparisons") {
        AIRequest r1("model-a");
        r1.add_user_message("Hello");
        AIRequest r2("model-a");
        r2.add_user_message("Hello");
        AIRequest r3("model-b");
        r3.add_user_message("Hello");

        REQUIRE(r1 == r2);
        REQUIRE_FALSE(r1 == r3);
    }
}

// ============================================================================
// 4. AIResponse Unit Tests
// ============================================================================

TEST_CASE("Usage: Default values and token calculation", "[tier1][domain][response]") {
    Usage u;
    REQUIRE(u.prompt_tokens == 0);
    REQUIRE(u.completion_tokens == 0);
    REQUIRE(u.total_tokens == 0);
    REQUIRE(u.is_zero());
    REQUIRE(u.empty());
    REQUIRE(u.effective_total_tokens() == 0);

    Usage u2{120, 35, 155};
    REQUIRE_FALSE(u2.is_zero());
    REQUIRE_FALSE(u2.empty());
    REQUIRE(u2.effective_total_tokens() == 155);
    REQUIRE(u2.prompt_tokens == 120);
    REQUIRE(u2.completion_tokens == 35);

    // Fallback total calculation when total_tokens == 0
    Usage u3{50, 25, 0};
    REQUIRE(u3.effective_total_tokens() == 75);

    // Equality operator
    Usage u4{120, 35, 155};
    REQUIRE(u2 == u4);
    REQUIRE_FALSE(u2 == u3);

    // Compatibility alias
    AIUsage ai_usage{10, 20, 30};
    REQUIRE(ai_usage.effective_total_tokens() == 30);
}

TEST_CASE("AIResponse - Success response construction and token usage calculations", "[tier1][domain][response]") {
    SECTION("Successful response with complete metadata") {
        Usage usage{150, 42, 192};
        auto resp = AIResponse::make_success(
            "Taxonomy: Personal/Finance/2026",
            "llama-3.2-vision",
            usage,
            "stop",
            200,
            184.2
        );

        REQUIRE(resp.success);
        REQUIRE(resp.is_success());
        REQUIRE_FALSE(resp.has_error());
        REQUIRE(resp.content == "Taxonomy: Personal/Finance/2026");
        REQUIRE(resp.text() == resp.content);
        REQUIRE(resp.model == "llama-3.2-vision");
        REQUIRE(resp.usage.prompt_tokens == 150);
        REQUIRE(resp.usage.completion_tokens == 42);
        REQUIRE(resp.usage.total_tokens == 192);
        REQUIRE(resp.usage.effective_total_tokens() == 192);
        REQUIRE(resp.finish_reason == "stop");
        REQUIRE(resp.http_status == 200);
        REQUIRE(resp.latency_ms == 184.2);
        REQUIRE_FALSE(resp.error_message.has_value());
    }

    SECTION("Usage struct arithmetic consistency and default initialization") {
        Usage u{300, 200, 500};
        REQUIRE(u.prompt_tokens + u.completion_tokens == u.total_tokens);

        Usage default_u;
        REQUIRE(default_u.prompt_tokens == 0);
        REQUIRE(default_u.completion_tokens == 0);
        REQUIRE(default_u.total_tokens == 0);
    }
}

TEST_CASE("AIResponse - Error responses and HTTP status classification helpers", "[tier1][tier2][domain][response]") {
    SECTION("Generic error response instantiation") {
        auto err = AIResponse::make_error("Host unreachable", 0, 15.0);

        REQUIRE_FALSE(err.success);
        REQUIRE_FALSE(err.is_success());
        REQUIRE(err.has_error());
        REQUIRE(err.content.empty());
        REQUIRE(err.http_status == 0);
        REQUIRE(err.error_message.has_value());
        REQUIRE(err.error_message.value() == "Host unreachable");
        REQUIRE(err.error_text_or() == "Host unreachable");
        REQUIRE(err.latency_ms == 15.0);
    }

    SECTION("make_failure factory parity") {
        auto fail = AIResponse::make_failure("Timeout occurred", 408, 30000.0, "gpt-4o");
        REQUIRE_FALSE(fail.success);
        REQUIRE(fail.http_status == 408);
        REQUIRE(fail.latency_ms == 30000.0);
        REQUIRE(fail.model == "gpt-4o");
        REQUIRE(fail.error_text_or("fallback") == "Timeout occurred");
    }

    SECTION("HTTP 429 Rate Limit classification") {
        auto rate_limited = AIResponse::make_error("Too Many Requests", 429);
        REQUIRE(rate_limited.is_rate_limited());
        REQUIRE_FALSE(rate_limited.is_auth_error());
        REQUIRE_FALSE(rate_limited.is_server_error());
    }

    SECTION("HTTP 401 & 403 Authentication / Authorization classification") {
        auto unauth = AIResponse::make_error("Invalid API Key", 401);
        REQUIRE(unauth.is_auth_error());
        REQUIRE_FALSE(unauth.is_rate_limited());
        REQUIRE_FALSE(unauth.is_server_error());

        auto forbidden = AIResponse::make_error("Access Forbidden", 403);
        REQUIRE(forbidden.is_auth_error());
    }

    SECTION("HTTP 500, 502, 503, 504 Server Error classification") {
        for (long code : {500, 502, 503, 504}) {
            auto server_err = AIResponse::make_error("Server failure", code);
            REQUIRE(server_err.is_server_error());
            REQUIRE_FALSE(server_err.is_auth_error());
            REQUIRE_FALSE(server_err.is_rate_limited());
        }
    }

    SECTION("HTTP 200 Success does not trigger error predicates") {
        Usage u{10, 10, 20};
        auto ok_resp = AIResponse::make_success("Category", "m", u, "stop", 200);

        REQUIRE_FALSE(ok_resp.is_rate_limited());
        REQUIRE_FALSE(ok_resp.is_auth_error());
        REQUIRE_FALSE(ok_resp.is_server_error());
    }

    SECTION("Default error text fallback") {
        AIResponse resp;
        REQUIRE(resp.error_text_or("Fallback Error") == "Fallback Error");
    }
}

TEST_CASE("AIResponse - Finish reasons and boundary content", "[tier2][domain][response]") {
    SECTION("Finish reason handling (stop, length, content_filter)") {
        Usage u;
        auto r_stop = AIResponse::make_success("done", "m", u, "stop");
        REQUIRE(r_stop.finish_reason == "stop");

        auto r_len = AIResponse::make_success("cut off...", "m", u, "length");
        REQUIRE(r_len.finish_reason == "length");

        auto r_filter = AIResponse::make_success("", "m", u, "content_filter");
        REQUIRE(r_filter.finish_reason == "content_filter");
    }

    SECTION("Empty content with success = true") {
        Usage u;
        auto empty_resp = AIResponse::make_success("", "model", u);
        REQUIRE(empty_resp.success);
        REQUIRE(empty_resp.content.empty());
    }

    SECTION("Equality operator") {
        auto r1 = AIResponse::make_success("ok", "m1");
        auto r2 = AIResponse::make_success("ok", "m1");
        auto r3 = AIResponse::make_success("different", "m1");

        REQUIRE(r1 == r2);
        REQUIRE_FALSE(r1 == r3);
    }
}

// ============================================================================
// 5. ProviderCapabilities Unit Tests
// ============================================================================

TEST_CASE("CapabilityStatus: String conversion helpers", "[tier1][domain][capabilities]") {
    REQUIRE(to_string(CapabilityStatus::Supported) == "Supported");
    REQUIRE(to_string(CapabilityStatus::NotSupported) == "NotSupported");
    REQUIRE(to_string(CapabilityStatus::Unknown) == "Unknown");

    REQUIRE(capability_status_to_string(CapabilityStatus::Supported) == "Supported");
    REQUIRE(capability_status_to_string(CapabilityStatus::NotSupported) == "NotSupported");
    REQUIRE(capability_status_to_string(CapabilityStatus::Unknown) == "Unknown");

    REQUIRE(capability_status_from_string("Supported") == CapabilityStatus::Supported);
    REQUIRE(capability_status_from_string("supported") == CapabilityStatus::Supported);
    REQUIRE(capability_status_from_string("true") == CapabilityStatus::Supported);
    REQUIRE(capability_status_from_string("1") == CapabilityStatus::Supported);

    REQUIRE(capability_status_from_string("NotSupported") == CapabilityStatus::NotSupported);
    REQUIRE(capability_status_from_string("not_supported") == CapabilityStatus::NotSupported);
    REQUIRE(capability_status_from_string("Unsupported") == CapabilityStatus::NotSupported);
    REQUIRE(capability_status_from_string("unsupported") == CapabilityStatus::NotSupported);
    REQUIRE(capability_status_from_string("false") == CapabilityStatus::NotSupported);
    REQUIRE(capability_status_from_string("0") == CapabilityStatus::NotSupported);

    REQUIRE(capability_status_from_string("Unknown") == CapabilityStatus::Unknown);
    REQUIRE(capability_status_from_string("unknown") == CapabilityStatus::Unknown);
    REQUIRE_FALSE(capability_status_from_string("invalid_status").has_value());
}

TEST_CASE("ProviderCapabilities - Default values and status evaluation", "[tier1][domain][capabilities]") {
    SECTION("Default constructor state") {
        ProviderCapabilities caps;
        REQUIRE_FALSE(caps.endpoint_reachable);
        REQUIRE(caps.latency_ms == -1);
        REQUIRE_FALSE(caps.supports_chat_completions);
        REQUIRE(caps.vision_capability == CapabilityStatus::Unknown);
        REQUIRE_FALSE(caps.supports_json_mode);
        REQUIRE_FALSE(caps.supports_model_discovery);
        REQUIRE(caps.available_models.empty());
        REQUIRE(caps.detected_server_flavor.empty());
        REQUIRE_FALSE(caps.is_functional());
        REQUIRE_FALSE(caps.is_healthy());
        REQUIRE_FALSE(caps.supports_vision());
    }

    SECTION("CapabilityStatus enum queries and supports_vision() helper") {
        ProviderCapabilities caps;

        caps.vision_capability = CapabilityStatus::Supported;
        REQUIRE(caps.supports_vision());

        caps.vision_capability = CapabilityStatus::NotSupported;
        REQUIRE_FALSE(caps.supports_vision());

        caps.vision_capability = CapabilityStatus::Unknown;
        REQUIRE_FALSE(caps.supports_vision());
    }
}

TEST_CASE("ProviderCapabilities - Model listing discovery, inspection, and summary", "[tier1][tier2][domain][capabilities]") {
    SECTION("Model lookup with has_model() helper") {
        ProviderCapabilities caps;
        caps.available_models = {
            "llama3.2-vision:11b",
            "mistral-nemo:12b",
            "qwen2.5:7b"
        };

        REQUIRE(caps.has_model("llama3.2-vision:11b"));
        REQUIRE(caps.has_model("mistral-nemo:12b"));
        REQUIRE(caps.has_model("qwen2.5:7b"));

        REQUIRE_FALSE(caps.has_model("gpt-4o"));
        REQUIRE_FALSE(caps.has_model(""));
        REQUIRE_FALSE(caps.has_model("llama3.2-vision")); // Exact match required
    }

    SECTION("Server flavor tagging and health evaluation") {
        ProviderCapabilities healthy_caps;
        healthy_caps.endpoint_reachable = true;
        healthy_caps.supports_chat_completions = true;
        healthy_caps.latency_ms = 35;
        healthy_caps.detected_server_flavor = "ollama";

        REQUIRE(healthy_caps.is_functional());
        REQUIRE(healthy_caps.is_healthy());
        REQUIRE(healthy_caps.detected_server_flavor == "ollama");

        ProviderCapabilities unreachable_caps;
        unreachable_caps.endpoint_reachable = false;
        unreachable_caps.supports_chat_completions = true;
        REQUIRE_FALSE(unreachable_caps.is_functional());
        REQUIRE_FALSE(unreachable_caps.is_healthy());

        ProviderCapabilities no_chat_caps;
        no_chat_caps.endpoint_reachable = true;
        no_chat_caps.supports_chat_completions = false;
        REQUIRE_FALSE(no_chat_caps.is_functional());
        REQUIRE_FALSE(no_chat_caps.is_healthy());
    }

    SECTION("Summary text formatting for unreachable endpoint") {
        ProviderCapabilities caps;
        REQUIRE(caps.summary_text() == "Endpoint unreachable");

        caps.raw_status_message = "Connection refused on port 11434";
        REQUIRE(caps.summary_text() == "Endpoint unreachable: Connection refused on port 11434");
    }

    SECTION("Summary text formatting for live endpoint") {
        ProviderCapabilities caps;
        caps.endpoint_reachable = true;
        caps.latency_ms = 42;
        caps.supports_chat_completions = true;
        caps.vision_capability = CapabilityStatus::Supported;
        caps.supports_json_mode = true;
        caps.supports_model_discovery = true;
        caps.available_models = {"llama3.2-vision", "qwen2.5-coder", "mistral-small"};
        caps.detected_server_flavor = "ollama";

        std::string summary = caps.summary_text();
        REQUIRE(summary.find("ollama") != std::string::npos);
        REQUIRE(summary.find("Chat: OK") != std::string::npos);
        REQUIRE(summary.find("Vision: Supported") != std::string::npos);
        REQUIRE(summary.find("42ms") != std::string::npos);
        REQUIRE(summary.find("Models: 3") != std::string::npos);
    }

    SECTION("Equality operator") {
        ProviderCapabilities c1;
        c1.endpoint_reachable = true;
        c1.latency_ms = 50;

        ProviderCapabilities c2;
        c2.endpoint_reachable = true;
        c2.latency_ms = 50;

        ProviderCapabilities c3;
        c3.endpoint_reachable = false;

        REQUIRE(c1 == c2);
        REQUIRE_FALSE(c1 == c3);
    }
}
