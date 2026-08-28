#include <catch2/catch_test_macros.hpp>

#include "OpenAICompatibleProvider.hpp"

#if __has_include(<jsoncpp/json/json.h>)
    #include <jsoncpp/json/json.h>
#elif __has_include(<json/json.h>)
    #include <json/json.h>
#endif

#include <sstream>

TEST_CASE("OpenAICompatibleProvider - Request Serialization", "[tier1][provider]") {
    SECTION("Text-only single message serialization") {
        AIRequest req("llama3.2:latest");
        req.add_system_message("You are a helpful file categorization assistant.");
        req.add_user_message("Categorize tax_2026.pdf");
        req.with_temperature(0.2f);
        req.with_max_tokens(256);

        std::string json_str = OpenAICompatibleProvider::serialize_chat_request(req, "default-model");

        Json::CharReaderBuilder reader;
        Json::Value root;
        std::string errs;
        std::istringstream stream(json_str);
        REQUIRE(Json::parseFromStream(reader, stream, &root, &errs));

        REQUIRE(root["model"].asString() == "llama3.2:latest");
        REQUIRE(root["temperature"].asFloat() == 0.2f);
        REQUIRE(root["max_tokens"].asInt() == 256);
        REQUIRE(root["messages"].isArray());
        REQUIRE(root["messages"].size() == 2);
        REQUIRE(root["messages"][0]["role"].asString() == "system");
        REQUIRE(root["messages"][0]["content"].asString() == "You are a helpful file categorization assistant.");
        REQUIRE(root["messages"][1]["role"].asString() == "user");
        REQUIRE(root["messages"][1]["content"].asString() == "Categorize tax_2026.pdf");
    }

    SECTION("Multimodal image + text message serialization") {
        AIRequest req("llava-v1.6-7b");
        const std::vector<uint8_t> dummy_jpg = {0xFF, 0xD8, 0xFF, 0xE0};
        req.add_user_multimodal_message("Categorize this receipt", dummy_jpg, "image/jpeg", "high");

        std::string json_str = OpenAICompatibleProvider::serialize_chat_request(req, "default-model");

        Json::CharReaderBuilder reader;
        Json::Value root;
        std::string errs;
        std::istringstream stream(json_str);
        REQUIRE(Json::parseFromStream(reader, stream, &root, &errs));

        REQUIRE(root["messages"].size() == 1);
        const auto& user_msg = root["messages"][0];
        REQUIRE(user_msg["role"].asString() == "user");
        REQUIRE(user_msg["content"].isArray());
        REQUIRE(user_msg["content"].size() == 2);

        REQUIRE(user_msg["content"][0]["type"].asString() == "text");
        REQUIRE(user_msg["content"][0]["text"].asString() == "Categorize this receipt");

        REQUIRE(user_msg["content"][1]["type"].asString() == "image_url");
        REQUIRE(user_msg["content"][1]["image_url"]["url"].asString() == "data:image/jpeg;base64,/9j/4A==");
        REQUIRE(user_msg["content"][1]["image_url"]["detail"].asString() == "high");
    }

    SECTION("JSON response format serialization") {
        AIRequest req;
        req.with_response_format("json_object");
        std::string json_str = OpenAICompatibleProvider::serialize_chat_request(req, "gpt-4o");

        Json::CharReaderBuilder reader;
        Json::Value root;
        std::string errs;
        std::istringstream stream(json_str);
        REQUIRE(Json::parseFromStream(reader, stream, &root, &errs));

        REQUIRE(root.isMember("response_format"));
        REQUIRE(root["response_format"]["type"].asString() == "json_object");
    }
}

TEST_CASE("OpenAICompatibleProvider - Response Deserialization", "[tier1][provider]") {
    SECTION("Successful standard OpenAI response") {
        const std::string payload = R"({
            "id": "chatcmpl-123",
            "model": "qwen2.5:7b",
            "choices": [
                {
                    "index": 0,
                    "message": {
                        "role": "assistant",
                        "content": "{"category": "Finance", "subcategory": "Tax"}"
                    },
                    "finish_reason": "stop"
                }
            ],
            "usage": {
                "prompt_tokens": 42,
                "completion_tokens": 18,
                "total_tokens": 60
            }
        })";

        AIResponse res = OpenAICompatibleProvider::deserialize_chat_response(payload, 200, 150);

        REQUIRE(res.is_success());
        REQUIRE(res.text() == "{"category": "Finance", "subcategory": "Tax"}");
        REQUIRE(res.model == "qwen2.5:7b");
        REQUIRE(res.finish_reason == "stop");
        REQUIRE(res.http_status == 200);
        REQUIRE(res.latency_ms == 150);
        REQUIRE(res.usage.prompt_tokens == 42);
        REQUIRE(res.usage.completion_tokens == 18);
        REQUIRE(res.usage.total_tokens == 60);
    }

    SECTION("HTTP 401 Unauthorized error parsing") {
        const std::string err_payload = R"({
            "error": {
                "message": "Incorrect API key provided",
                "type": "invalid_request_error"
            }
        })";

        AIResponse res = OpenAICompatibleProvider::deserialize_chat_response(err_payload, 401, 50);

        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.has_error());
        REQUIRE(res.is_auth_error());
        REQUIRE(res.http_status == 401);
        REQUIRE(res.error_message.find("Incorrect API key") != std::string::npos);
    }

    SECTION("HTTP 429 Rate Limit error parsing") {
        const std::string err_payload = R"({"error": "Rate limit reached"})";
        AIResponse res = OpenAICompatibleProvider::deserialize_chat_response(err_payload, 429, 30);

        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.is_rate_limited());
        REQUIRE(res.http_status == 429);
    }
}

TEST_CASE("OpenAICompatibleProvider - Model Discovery Parsing", "[tier1][provider]") {
    SECTION("Parse OpenAI-format model list") {
        const std::string payload = R"({
            "data": [
                { "id": "gpt-4o" },
                { "id": "text-embedding-3-small" },
                { "id": "gpt-4o-mini" }
            ]
        })";

        auto models = OpenAICompatibleProvider::parse_models_list(payload);
        REQUIRE(models.size() == 3);
        REQUIRE(models[0] == "gpt-4o");
        REQUIRE(models[1] == "gpt-4o-mini");
        REQUIRE(models[2] == "text-embedding-3-small");
    }

    SECTION("Parse Ollama-format model list") {
        const std::string payload = R"({
            "models": [
                { "name": "llama3.2:latest" },
                { "name": "qwen2.5-coder:7b" }
            ]
        })";

        auto models = OpenAICompatibleProvider::parse_models_list(payload);
        REQUIRE(models.size() == 2);
        REQUIRE(models[0] == "llama3.2:latest");
        REQUIRE(models[1] == "qwen2.5-coder:7b");
    }
}

TEST_CASE("OpenAICompatibleProvider - Mock HTTP Transport Execution", "[tier2][provider]") {
    SECTION("Mocked successful response dispatch") {
        OpenAICompatibleProvider::Config cfg;
        cfg.endpoint_url = "http://localhost:11434/v1";
        cfg.default_model = "qwen2.5:7b";
        cfg.api_key = "secret_key_12345";

        OpenAICompatibleProvider provider(cfg);

        // Inject mock transport
        provider.set_http_transport_for_testing([](const std::string& url,
                                                   const std::vector<std::string>& headers,
                                                   const std::string& payload,
                                                   std::chrono::milliseconds timeout) {
            REQUIRE(url == "http://localhost:11434/v1/chat/completions");

            // Verify Authorization header presence
            bool has_auth = false;
            for (const auto& h : headers) {
                if (h == "Authorization: Bearer secret_key_12345") {
                    has_auth = true;
                }
            }
            REQUIRE(has_auth);

            OpenAICompatibleProvider::HttpResponse res;
            res.status_code = 200;
            res.latency_ms = 85;
            res.body = R"({
                "model": "qwen2.5:7b",
                "choices": [{
                    "message": { "role": "assistant", "content": "ENDPOINT_TEST_OK" }
                }]
            })";
            return res;
        });

        AIRequest req;
        req.add_user_message("Respond with exactly: ENDPOINT_TEST_OK");

        AIResponse res = provider.generate_response(req);
        REQUIRE(res.is_success());
        REQUIRE(res.text() == "ENDPOINT_TEST_OK");
        REQUIRE(res.model == "qwen2.5:7b");
        REQUIRE(res.latency_ms == 85);
    }
}
