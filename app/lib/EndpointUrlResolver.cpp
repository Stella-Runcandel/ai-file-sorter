#include "EndpointUrlResolver.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

std::string EndpointUrlResolver::trim_ws_and_slashes(std::string_view input) {
    const char* ws = " \t\n\r\f\v/";
    size_t first = input.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string_view::npos) {
        return {};
    }
    size_t last = input.find_last_not_of(" \t\n\r\f\v");
    std::string_view trimmed = input.substr(first, last - first + 1);

    // Strip trailing slashes
    while (trimmed.size() > 1 && trimmed.back() == '/') {
        // Prevent stripping http:// down to http:/
        if (trimmed.size() >= 3 && trimmed[trimmed.size() - 2] == ':' && trimmed.back() == '/') {
            break;
        }
        if (trimmed.size() >= 4 && trimmed[trimmed.size() - 3] == ':' &&
            trimmed[trimmed.size() - 2] == '/' && trimmed.back() == '/') {
            break;
        }
        trimmed.remove_suffix(1);
    }
    return std::string(trimmed);
}

bool EndpointUrlResolver::ends_with_ci(std::string_view str, std::string_view suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin(),
                      [](char a, char b) {
                          return std::tolower(static_cast<unsigned char>(a)) ==
                                 std::tolower(static_cast<unsigned char>(b));
                      });
}

std::string EndpointUrlResolver::resolve_chat_completions_url(std::string_view input_url) {
    std::string trimmed = trim_ws_and_slashes(input_url);
    if (trimmed.empty()) {
        return std::string(kDefaultOpenAIChatEndpoint);
    }

    if (ends_with_ci(trimmed, "/v1/chat/completions")) {
        return trimmed;
    }
    if (ends_with_ci(trimmed, "/chat/completions")) {
        return trimmed;
    }
    if (ends_with_ci(trimmed, "/v1")) {
        return trimmed + "/chat/completions";
    }

    // Root server URL (e.g. http://localhost:11434, http://localhost:1234, https://api.openai.com)
    return trimmed + "/v1/chat/completions";
}

std::string EndpointUrlResolver::resolve_models_url(std::string_view input_url) {
    std::string trimmed = trim_ws_and_slashes(input_url);
    if (trimmed.empty()) {
        return std::string(kDefaultOpenAIModelsEndpoint);
    }

    if (ends_with_ci(trimmed, "/v1/chat/completions")) {
        return trimmed.substr(0, trimmed.size() - 17) + "/models";
    }
    if (ends_with_ci(trimmed, "/chat/completions")) {
        return trimmed.substr(0, trimmed.size() - 17) + "/models";
    }
    if (ends_with_ci(trimmed, "/v1/models") || ends_with_ci(trimmed, "/models")) {
        return trimmed;
    }
    if (ends_with_ci(trimmed, "/v1")) {
        return trimmed + "/models";
    }

    return trimmed + "/v1/models";
}

std::string EndpointUrlResolver::extract_host_display(std::string_view input_url) {
    std::string trimmed = trim_ws_and_slashes(input_url);
    if (trimmed.empty()) {
        return "api.openai.com";
    }

    // Strip scheme
    size_t scheme_pos = trimmed.find("://");
    std::string_view without_scheme = (scheme_pos != std::string::npos)
                                          ? std::string_view(trimmed).substr(scheme_pos + 3)
                                          : std::string_view(trimmed);

    // Strip path
    size_t path_pos = without_scheme.find('/');
    if (path_pos != std::string_view::npos) {
        without_scheme = without_scheme.substr(0, path_pos);
    }

    // Strip userinfo if present
    size_t at_pos = without_scheme.find('@');
    if (at_pos != std::string_view::npos) {
        without_scheme = without_scheme.substr(at_pos + 1);
    }

    return std::string(without_scheme);
}

std::string EndpointUrlResolver::sanitize_url_for_logging(std::string_view input_url) {
    std::string url = trim_ws_and_slashes(input_url);
    if (url.empty()) {
        return {};
    }

    size_t scheme_pos = url.find("://");
    if (scheme_pos == std::string::npos) {
        return url;
    }

    size_t at_pos = url.find('@', scheme_pos + 3);
    size_t slash_pos = url.find('/', scheme_pos + 3);

    // If @ is before the first path slash, credentials exist
    if (at_pos != std::string::npos && (slash_pos == std::string::npos || at_pos < slash_pos)) {
        return url.substr(0, scheme_pos + 3) + url.substr(at_pos + 1);
    }

    return url;
}
