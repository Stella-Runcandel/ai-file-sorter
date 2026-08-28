#include "OpenAICompatibleProvider.hpp"
#include "Base64.hpp"
#include "EndpointUrlResolver.hpp"
#include "Logger.hpp"

#include <curl/curl.h>

#if __has_include(<jsoncpp/json/json.h>)
    #include <jsoncpp/json/json.h>
#elif __has_include(<json/json.h>)
    #include <json/json.h>
#else
    #error "jsoncpp headers not found. Install jsoncpp development files."
#endif

#include <algorithm>
#include <chrono>
#include <cctype>
#include <sstream>
#include <utility>

namespace {

size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_bytes = size * nmemb;
    if (userp) {
        userp->append(static_cast<const char*>(contents), total_bytes);
    }
    return total_bytes;
}

} // namespace

OpenAICompatibleProvider::OpenAICompatibleProvider(Config config)
    : m_config(std::move(config)) {}

OpenAICompatibleProvider::OpenAICompatibleProvider(std::string endpoint_url,
                                                   std::string api_key,
                                                   std::string default_model) {
    m_config.endpoint_url = std::move(endpoint_url);
    m_config.api_key = std::move(api_key);
    m_config.default_model = std::move(default_model);
}

std::string OpenAICompatibleProvider::mask_api_key(const std::string& key) {
    if (key.empty()) {
        return "(none)";
    }
    if (key.size() <= 8) {
        return "********";
    }
    return key.substr(0, 4) + "..." + key.substr(key.size() - 4);
}

std::string OpenAICompatibleProvider::serialize_chat_request(const AIRequest& request,
                                                             const std::string& fallback_model) {
    Json::Value root(Json::objectValue);

    // Model selection
    std::string model_name = !request.model.empty() ? request.model : fallback_model;
    root["model"] = model_name;

    // Sampling parameters
    if (request.temperature.has_value()) {
        root["temperature"] = *request.temperature;
    }
    if (request.max_tokens.has_value()) {
        root["max_tokens"] = *request.max_tokens;
    }
    if (request.response_format.has_value()) {
        if (*request.response_format == "json" || *request.response_format == "json_object") {
            Json::Value format_obj(Json::objectValue);
            format_obj["type"] = "json_object";
            root["response_format"] = format_obj;
        }
    }

    // Messages array
    Json::Value messages_array(Json::arrayValue);
    for (const auto& msg : request.messages) {
        Json::Value msg_obj(Json::objectValue);
        msg_obj["role"] = std::string(to_string(msg.role));

        if (!msg.is_multimodal()) {
            // Standard single-part text message
            msg_obj["content"] = msg.text_content();
        } else {
            // Multimodal content array
            Json::Value content_array(Json::arrayValue);
            for (const auto& part : msg.parts) {
                if (part.is_text()) {
                    Json::Value text_part(Json::objectValue);
                    text_part["type"] = "text";
                    text_part["text"] = part.text();
                    content_array.append(text_part);
                } else if (part.is_image()) {
                    const auto& img = part.image();
                    Json::Value img_part(Json::objectValue);
                    img_part["type"] = "image_url";

                    Json::Value img_url_obj(Json::objectValue);
                    img_url_obj["url"] = Base64::to_data_uri(img.bytes, img.mime_type);
                    if (img.detail.has_value()) {
                        img_url_obj["detail"] = *img.detail;
                    }
                    img_part["image_url"] = img_url_obj;
                    content_array.append(img_part);
                }
            }
            msg_obj["content"] = content_array;
        }

        messages_array.append(msg_obj);
    }
    root["messages"] = messages_array;

    Json::StreamWriterBuilder writer;
    writer["indentation"] = ""; // Compact JSON
    return Json::writeString(writer, root);
}

AIResponse OpenAICompatibleProvider::deserialize_chat_response(const std::string& json_response,
                                                               long http_status,
                                                               long latency_ms) {
    if (http_status < 200 || http_status >= 300) {
        std::string error_detail;
        Json::CharReaderBuilder reader;
        Json::Value root;
        std::string errs;
        std::istringstream stream(json_response);
        if (Json::parseFromStream(reader, stream, &root, &errs)) {
            if (root.isMember("error")) {
                if (root["error"].isObject() && root["error"].isMember("message")) {
                    error_detail = root["error"]["message"].asString();
                } else if (root["error"].isString()) {
                    error_detail = root["error"].asString();
                }
            }
        }
        if (error_detail.empty()) {
            error_detail = json_response.empty() ? "HTTP error" : json_response;
        }
        return AIResponse::make_failure("HTTP " + std::to_string(http_status) + ": " + error_detail,
                                        http_status, latency_ms);
    }

    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream stream(json_response);
    if (!Json::parseFromStream(reader, stream, &root, &errs)) {
        return AIResponse::make_failure("Invalid JSON response from endpoint: " + errs,
                                        http_status, latency_ms);
    }

    if (!root.isMember("choices") || !root["choices"].isArray() || root["choices"].empty()) {
        return AIResponse::make_failure("Malformed response: missing choices array",
                                        http_status, latency_ms);
    }

    const auto& first_choice = root["choices"][0];
    std::string generated_text;

    if (first_choice.isMember("message") && first_choice["message"].isMember("content")) {
        generated_text = first_choice["message"]["content"].asString();
    } else if (first_choice.isMember("text")) {
        generated_text = first_choice["text"].asString();
    }

    std::string model_name = root.isMember("model") ? root["model"].asString() : "";
    std::string finish_reason = first_choice.isMember("finish_reason") && first_choice["finish_reason"].isString()
                                    ? first_choice["finish_reason"].asString()
                                    : "stop";

    AIResponse::Usage usage;
    if (root.isMember("usage") && root["usage"].isObject()) {
        const auto& u = root["usage"];
        if (u.isMember("prompt_tokens") && u["prompt_tokens"].isInt()) {
            usage.prompt_tokens = u["prompt_tokens"].asInt();
        }
        if (u.isMember("completion_tokens") && u["completion_tokens"].isInt()) {
            usage.completion_tokens = u["completion_tokens"].asInt();
        }
        if (u.isMember("total_tokens") && u["total_tokens"].isInt()) {
            usage.total_tokens = u["total_tokens"].asInt();
        }
    }

    AIResponse response = AIResponse::make_success(std::move(generated_text),
                                                   std::move(model_name),
                                                   usage,
                                                   std::move(finish_reason),
                                                   http_status,
                                                   static_cast<double>(latency_ms));
    return response;
}

std::vector<std::string> OpenAICompatibleProvider::parse_models_list(const std::string& json_response) {
    std::vector<std::string> models;
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream stream(json_response);
    if (!Json::parseFromStream(reader, stream, &root, &errs)) {
        return models;
    }

    // Standard OpenAI schema: { "data": [ { "id": "model-name" }, ... ] }
    if (root.isMember("data") && root["data"].isArray()) {
        for (const auto& item : root["data"]) {
            if (item.isMember("id") && item["id"].isString()) {
                models.push_back(item["id"].asString());
            }
        }
    }
    // Ollama native schema: { "models": [ { "name": "model:tag" }, ... ] }
    else if (root.isMember("models") && root["models"].isArray()) {
        for (const auto& item : root["models"]) {
            if (item.isMember("name") && item["name"].isString()) {
                models.push_back(item["name"].asString());
            } else if (item.isMember("model") && item["model"].isString()) {
                models.push_back(item["model"].asString());
            }
        }
    }

    std::sort(models.begin(), models.end());
    models.erase(std::unique(models.begin(), models.end()), models.end());
    return models;
}

std::vector<std::string> OpenAICompatibleProvider::build_headers() const {
    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Accept: application/json");
    if (!m_config.api_key.empty()) {
        headers.push_back("Authorization: Bearer " + m_config.api_key);
    }
    return headers;
}

OpenAICompatibleProvider::HttpResponse OpenAICompatibleProvider::perform_curl_request(
    const std::string& url,
    const std::vector<std::string>& headers,
    const std::string& payload,
    bool is_post,
    std::chrono::milliseconds timeout) {

    HttpResponse res;
    CURL* curl = curl_easy_init();
    if (!curl) {
        res.error_message = "Failed to initialize libcurl easy handle";
        return res;
    }

    struct curl_slist* header_list = nullptr;
    for (const auto& h : headers) {
        header_list = curl_slist_append(header_list, h.c_str());
    }

    std::string response_body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    long timeout_ms = static_cast<long>(timeout.count());
    if (timeout_ms <= 0) timeout_ms = 60000;
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, std::min(timeout_ms, 15000L));

    if (is_post) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    } else {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }

    auto start_time = std::chrono::steady_clock::now();
    CURLcode code = curl_easy_perform(curl);
    auto end_time = std::chrono::steady_clock::now();

    res.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if (code == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status_code);
        res.body = std::move(response_body);
    } else {
        res.status_code = (code == CURLE_OPERATION_TIMEDOUT) ? 408 : 0;
        res.error_message = curl_easy_strerror(code);
    }

    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    return res;
}

OpenAICompatibleProvider::HttpResponse OpenAICompatibleProvider::execute_http_post(
    const std::string& url,
    const std::string& payload,
    std::chrono::milliseconds timeout) const {
    auto headers = build_headers();
    if (m_custom_transport) {
        return m_custom_transport(url, headers, payload, timeout);
    }
    return perform_curl_request(url, headers, payload, true, timeout);
}

OpenAICompatibleProvider::HttpResponse OpenAICompatibleProvider::execute_http_get(
    const std::string& url,
    std::chrono::milliseconds timeout) const {
    auto headers = build_headers();
    if (m_custom_transport) {
        return m_custom_transport(url, headers, "", timeout);
    }
    return perform_curl_request(url, headers, "", false, timeout);
}

AIResponse OpenAICompatibleProvider::generate_response(const AIRequest& request) {
    std::string target_url = EndpointUrlResolver::resolve_chat_completions_url(m_config.endpoint_url);
    std::string payload = serialize_chat_request(request, m_config.default_model);

    auto timeout = (request.timeout.count() > 0) ? request.timeout : m_config.timeout;

    HttpResponse http_res = execute_http_post(target_url, payload, timeout);

    if (http_res.status_code == 0) {
        return AIResponse::make_failure("Network transport failure: " + http_res.error_message,
                                        0, http_res.latency_ms);
    }

    return deserialize_chat_response(http_res.body, http_res.status_code, http_res.latency_ms);
}

std::vector<std::string> OpenAICompatibleProvider::fetch_available_models() {
    std::string models_url = EndpointUrlResolver::resolve_models_url(m_config.endpoint_url);
    HttpResponse http_res = execute_http_get(models_url, std::chrono::milliseconds(10000));
    if (http_res.status_code >= 200 && http_res.status_code < 300) {
        return parse_models_list(http_res.body);
    }
    return {};
}

ProviderCapabilities OpenAICompatibleProvider::probe_endpoint() {
    ProviderCapabilities caps;
    std::string models_url = EndpointUrlResolver::resolve_models_url(m_config.endpoint_url);

    auto start_time = std::chrono::steady_clock::now();
    HttpResponse http_res = execute_http_get(models_url, std::chrono::milliseconds(10000));
    auto end_time = std::chrono::steady_clock::now();

    caps.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if (http_res.status_code >= 200 && http_res.status_code < 300) {
        caps.endpoint_reachable = true;
        caps.supports_chat_completions = true;
        caps.supports_model_discovery = true;
        caps.supports_json_mode = true;
        caps.available_models = parse_models_list(http_res.body);

        // Detect server flavor
        std::string host = EndpointUrlResolver::extract_host_display(m_config.endpoint_url);
        if (host.find("11434") != std::string::npos || host.find("ollama") != std::string::npos) {
            caps.detected_server_flavor = "Ollama";
        } else if (host.find("1234") != std::string::npos) {
            caps.detected_server_flavor = "LM Studio";
        } else if (host.find("8080") != std::string::npos) {
            caps.detected_server_flavor = "llama-server";
        } else if (host.find("openai.com") != std::string::npos) {
            caps.detected_server_flavor = "OpenAI";
        } else {
            caps.detected_server_flavor = "OpenAI-Compatible";
        }

        // Vision capability remains Unknown unless verified or indicated by model name
        std::string target_model = m_config.default_model;
        std::string lower_model = target_model;
        std::transform(lower_model.begin(), lower_model.end(), lower_model.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (lower_model.find("vision") != std::string::npos ||
            lower_model.find("vl") != std::string::npos ||
            lower_model.find("llava") != std::string::npos ||
            lower_model.find("gpt-4o") != std::string::npos ||
            lower_model.find("claude") != std::string::npos) {
            caps.vision_capability = CapabilityStatus::Supported;
        } else {
            caps.vision_capability = CapabilityStatus::Unknown;
        }

        caps.raw_status_message = "Connected to " + caps.detected_server_flavor;
    } else {
        caps.endpoint_reachable = false;
        caps.supports_chat_completions = false;
        caps.raw_status_message = http_res.error_message.empty()
                                      ? ("HTTP " + std::to_string(http_res.status_code))
                                      : http_res.error_message;
    }

    return caps;
}

ProviderCapabilities OpenAICompatibleProvider::get_capabilities() {
    return probe_endpoint();
}
