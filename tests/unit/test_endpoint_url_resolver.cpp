#include <catch2/catch_test_macros.hpp>

#include "EndpointUrlResolver.hpp"

TEST_CASE("EndpointUrlResolver - Chat Completions URL Resolution", "[tier1][url_resolver]") {
    SECTION("Empty input defaults to OpenAI chat completions endpoint") {
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("") ==
                "https://api.openai.com/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("   ") ==
                "https://api.openai.com/v1/chat/completions");
    }

    SECTION("Ollama root URL is normalized with /v1/chat/completions (Defect Fix)") {
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:11434") ==
                "http://localhost:11434/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:11434/") ==
                "http://localhost:11434/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://127.0.0.1:11434") ==
                "http://127.0.0.1:11434/v1/chat/completions");
    }

    SECTION("Endpoints with /v1 suffix") {
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:11434/v1") ==
                "http://localhost:11434/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:11434/v1/") ==
                "http://localhost:11434/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:1234/v1") ==
                "http://localhost:1234/v1/chat/completions");
    }

    SECTION("Fully qualified chat completions URLs remain unchanged") {
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:11434/v1/chat/completions") ==
                "http://localhost:11434/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:8080/v1/chat/completions/") ==
                "http://localhost:8080/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("http://localhost:11434/chat/completions") ==
                "http://localhost:11434/chat/completions");
    }

    SECTION("Cloud and custom provider endpoints") {
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("https://api.openai.com") ==
                "https://api.openai.com/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("https://api.openai.com/v1") ==
                "https://api.openai.com/v1/chat/completions");
        REQUIRE(EndpointUrlResolver::resolve_chat_completions_url("https://openrouter.ai/api/v1") ==
                "https://openrouter.ai/api/v1/chat/completions");
    }
}

TEST_CASE("EndpointUrlResolver - Models URL Resolution", "[tier1][url_resolver]") {
    SECTION("Empty input defaults to OpenAI models endpoint") {
        REQUIRE(EndpointUrlResolver::resolve_models_url("") ==
                "https://api.openai.com/v1/models");
    }

    SECTION("Root server URL resolution to /v1/models") {
        REQUIRE(EndpointUrlResolver::resolve_models_url("http://localhost:11434") ==
                "http://localhost:11434/v1/models");
        REQUIRE(EndpointUrlResolver::resolve_models_url("http://localhost:1234/v1") ==
                "http://localhost:1234/v1/models");
    }

    SECTION("Chat completions endpoint converts to /models") {
        REQUIRE(EndpointUrlResolver::resolve_models_url("http://localhost:11434/v1/chat/completions") ==
                "http://localhost:11434/v1/models");
        REQUIRE(EndpointUrlResolver::resolve_models_url("http://localhost:11434/chat/completions") ==
                "http://localhost:11434/models");
    }
}

TEST_CASE("EndpointUrlResolver - Display and Logging Sanitization", "[tier2][url_resolver]") {
    SECTION("Host extraction for display") {
        REQUIRE(EndpointUrlResolver::extract_host_display("http://localhost:11434/v1/chat/completions") ==
                "localhost:11434");
        REQUIRE(EndpointUrlResolver::extract_host_display("https://api.openai.com/v1") ==
                "api.openai.com");
        REQUIRE(EndpointUrlResolver::extract_host_display("") ==
                "api.openai.com");
    }

    SECTION("Credential stripping for log safety") {
        REQUIRE(EndpointUrlResolver::sanitize_url_for_logging("http://user:secret@localhost:11434/v1") ==
                "http://localhost:11434/v1");
        REQUIRE(EndpointUrlResolver::sanitize_url_for_logging("https://api.openai.com/v1") ==
                "https://api.openai.com/v1");
    }
}
