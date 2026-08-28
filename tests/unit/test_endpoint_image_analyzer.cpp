#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "EndpointImageAnalyzer.hpp"
#include "IAIProvider.hpp"
#include "AIRequest.hpp"
#include "AIResponse.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace {

class MockVisionProvider : public IAIProvider {
public:
    bool reject_images = false;
    bool return_malformed_json = false;
    mutable AIRequest last_request;

    AIResponse generate_response(const AIRequest& request) override {
        last_request = request;

        if (reject_images) {
            return AIResponse::make_failure("The selected model does not support image input", 400, 10.0, request.model);
        }

        if (return_malformed_json) {
            return AIResponse::make_success("This is a photo of a mountain peak during sunset.", request.model, {}, "stop", 200, 25.0);
        }

        const std::string json_reply =
            R"({"description": "A close-up photograph of a red sports car on a race track.", "suggested_name": "red_sports_car_track.jpg"})";
        return AIResponse::make_success(json_reply, request.model, {}, "stop", 200, 30.0);
    }

    ProviderCapabilities get_capabilities() override {
        ProviderCapabilities caps;
        caps.endpoint_reachable = true;
        caps.supports_chat_completions = true;
        caps.vision_capability = reject_images ? CapabilityStatus::NotSupported : CapabilityStatus::Supported;
        return caps;
    }

    std::string provider_name() const override {
        return "MockVisionProvider";
    }
};

std::filesystem::path create_temporary_image_file() {
    const auto temp_path = std::filesystem::temp_directory_path() / "test_endpoint_sample.jpg";
    // Write a dummy JPEG signature (FF D8 FF E0 ...)
    std::ofstream out(temp_path, std::ios::binary);
    const unsigned char dummy_jpeg[] = {
        0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,
        0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x00, 0xFF, 0xD9
    };
    out.write(reinterpret_cast<const char*>(dummy_jpeg), sizeof(dummy_jpeg));
    out.close();
    return temp_path;
}

} // namespace

TEST_CASE("EndpointImageAnalyzer - JSON Response Parsing and Filename Sanitization", "[ai][vision]") {
    auto mock_provider = std::make_shared<MockVisionProvider>();
    EndpointImageAnalyzer analyzer(mock_provider);

    const auto temp_image = create_temporary_image_file();
    REQUIRE(std::filesystem::exists(temp_image));

    const auto result = analyzer.analyze(temp_image);

    CHECK(result.description == "A close-up photograph of a red sports car on a race track.");
    // Notice extension .jpg was stripped and sanitized
    CHECK(result.suggested_name == "red_sports_car_track");
    CHECK(result.diagnostics.available);
    CHECK(result.diagnostics.total_ms > 0.0);

    // Verify the request sent to provider was multimodal
    CHECK(mock_provider->last_request.is_multimodal());
    CHECK(mock_provider->last_request.messages.size() == 2);


    std::filesystem::remove(temp_image);
}

TEST_CASE("EndpointImageAnalyzer - Plain Text Fallback", "[ai][vision]") {
    auto mock_provider = std::make_shared<MockVisionProvider>();
    mock_provider->return_malformed_json = true;
    EndpointImageAnalyzer analyzer(mock_provider);

    const auto temp_image = create_temporary_image_file();
    const auto result = analyzer.analyze(temp_image);

    CHECK(result.description == "This is a photo of a mountain peak during sunset.");
    CHECK(result.suggested_name.empty());

    std::filesystem::remove(temp_image);
}

TEST_CASE("EndpointImageAnalyzer - Rejection from Text-Only Model Throws Clean Error", "[ai][vision]") {
    auto mock_provider = std::make_shared<MockVisionProvider>();
    mock_provider->reject_images = true;
    EndpointImageAnalyzer analyzer(mock_provider);

    const auto temp_image = create_temporary_image_file();

    REQUIRE_THROWS_WITH(
        analyzer.analyze(temp_image),
        Catch::Matchers::ContainsSubstring("The selected AI model does not support image input")
    );

    std::filesystem::remove(temp_image);
}
