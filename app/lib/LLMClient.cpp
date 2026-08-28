#include "LLMClient.hpp"
#include "OpenAICompatibleProvider.hpp"
#include "EndpointUrlResolver.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "Logger.hpp"
#include "RemoteApiError.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace {

long resolve_custom_timeout_seconds() {
    const char* env = std::getenv("AI_FILE_SORTER_CUSTOM_LLM_TIMEOUT");
    if (env && *env) {
        char* end = nullptr;
        const long value = std::strtol(env, &end, 10);
        if (end != env && value > 0) {
            return value;
        }
    }
    return 60L;
}

long resolve_openai_timeout_seconds() {
    return 30L;
}

long resolve_timeout_seconds(const std::string& base_url) {
    if (base_url.empty()) {
        return resolve_openai_timeout_seconds();
    }
    return resolve_custom_timeout_seconds();
}

} // namespace

LLMClient::LLMClient(std::string api_key, std::string model, std::string base_url)
    : api_key_(std::move(api_key)),
      model_(std::move(model)),
      base_url_(std::move(base_url))
{
    OpenAICompatibleProvider::Config config;
    config.endpoint_url = base_url_;
    config.api_key = api_key_;
    config.default_model = effective_model();
    config.timeout = std::chrono::seconds(resolve_timeout_seconds(base_url_));
    provider_ = std::make_unique<OpenAICompatibleProvider>(std::move(config));
}

LLMClient::~LLMClient() = default;

void LLMClient::set_prompt_logging_enabled(bool enabled)
{
    prompt_logging_enabled_ = enabled;
}

std::string LLMClient::effective_model() const
{
    static const std::string kDefaultModel = "gpt-4o-mini";
    if (model_.empty()) {
        return kDefaultModel;
    }
    return model_;
}

std::string LLMClient::resolve_api_url() const
{
    return EndpointUrlResolver::resolve_chat_completions_url(base_url_);
}

std::string LLMClient::categorize_file(const std::string& file_name,
                                       const std::string& file_path,
                                       FileType file_type,
                                       const std::string& consistency_context)
{
    if (auto logger = Logger::get_logger("core_logger")) {
        if (!file_path.empty()) {
            logger->debug("Requesting remote categorization for '{}' ({}) at '{}'",
                          file_name, to_string(file_type), file_path);
        } else {
            logger->debug("Requesting remote categorization for '{}' ({})", file_name, to_string(file_type));
        }
    }

    std::string prompt;
    if (!file_path.empty()) {
        prompt = (file_type == FileType::Directory ? "Categorize the directory with full path: " : "Categorize the item with full path: ") + file_path + "\n";
        prompt += (file_type == FileType::Directory ? "Directory name: " : "File name: ") + file_name;
    } else {
        prompt = (file_type == FileType::Directory ? "Categorize directory: " : "Categorize file: ") + file_name;
    }

    if (!consistency_context.empty()) {
        prompt += "\n\n" + consistency_context;
    }

    last_prompt_ = prompt;

    const std::string system_prompt =
        "You are a file categorization assistant. If it's an installer, describe the type of software it installs. "
        "Consider the filename, extension, and any directory context provided. If the user prompt includes an "
        "'Allowed main categories' list, choose the main category from that list only. Use Other only when it is "
        "listed and none of the other listed main categories clearly fits. Always reply with one line in the "
        "format <Main category> : <Subcategory>. Main category must be broad (one or two words, plural). "
        "Subcategory must be specific, relevant, and must not repeat the main category.";

    AIRequest req(effective_model());
    req.add_system_message(system_prompt);
    req.add_user_message(prompt);
    req.with_temperature(0.2f);

    if (prompt_logging_enabled_ && !last_prompt_.empty()) {
        std::cout << "\n[DEV][PROMPT] Categorization request\n" << last_prompt_ << "\n";
    }

    const AIResponse resp = provider_->generate_response(req);

    if (resp.has_error()) {
        auto logger = Logger::get_logger("core_logger");
        RemoteApiError::throw_for_http_error(
            "Remote LLM",
            resp.http_status,
            resp.error_message.value_or("Remote categorization failed"),
            "",
            logger
        );
    }

    if (prompt_logging_enabled_) {
        std::cout << "[DEV][RESPONSE] Categorization reply\n" << resp.text() << "\n";
    }

    return resp.text();
}

std::string LLMClient::complete_prompt(const std::string& prompt, int max_tokens)
{
    static const std::string kSystem =
        "You are a precise assistant that returns well-formed JSON responses.";

    if (prompt_logging_enabled_) {
        std::cout << "\n[DEV][PROMPT] Completion request\n" << prompt << "\n";
    }

    AIRequest req(effective_model());
    req.add_system_message(kSystem);
    req.add_user_message(prompt);
    if (max_tokens > 0) {
        req.with_max_tokens(max_tokens);
    }

    const AIResponse resp = provider_->generate_response(req);

    if (resp.has_error()) {
        auto logger = Logger::get_logger("core_logger");
        RemoteApiError::throw_for_http_error(
            "Remote LLM",
            resp.http_status,
            resp.error_message.value_or("Remote completion failed"),
            "",
            logger
        );
    }

    if (prompt_logging_enabled_) {
        std::cout << "[DEV][RESPONSE] Completion reply\n" << resp.text() << "\n";
    }

    return resp.text();
}
