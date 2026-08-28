#include "EndpointImageAnalyzer.hpp"
#include "ImageDecodeUtils.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QString>

#if __has_include(<jsoncpp/json/json.h>)
    #include <jsoncpp/json/json.h>
#elif __has_include(<json/json.h>)
    #include <json/json.h>
#else
    #error "jsoncpp headers not found. Install jsoncpp development files."
#endif

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

constexpr int kMaxImageDimension = 2048;
constexpr int kJpegQuality = 85;

std::string sanitize_suggested_filename(std::string name) {
    while (!name.empty() && (name.front() == '"' || name.front() == '\'' || std::isspace(static_cast<unsigned char>(name.front())))) {
        name.erase(name.begin());
    }
    while (!name.empty() && (name.back() == '"' || name.back() == '\'' || std::isspace(static_cast<unsigned char>(name.back())))) {
        name.pop_back();
    }
    for (char& ch : name) {
        if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
            ch = '_';
        }
    }
    // Remove common file extension if model appended it
    const auto dot_pos = name.rfind('.');
    if (dot_pos != std::string::npos && dot_pos > 0) {
        std::string ext = name.substr(dot_pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".webp") {
            name = name.substr(0, dot_pos);
        }
    }
    return name;
}


std::pair<std::string, std::string> parse_vision_response(const std::string& raw_content) {
    std::string description;
    std::string suggested_name;

    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;

    // Try parsing as clean JSON first
    std::string json_str = raw_content;
    auto first_brace = json_str.find('{');
    auto last_brace = json_str.rfind('}');
    if (first_brace != std::string::npos && last_brace != std::string::npos && last_brace > first_brace) {
        json_str = json_str.substr(first_brace, last_brace - first_brace + 1);
    }

    std::istringstream stream(json_str);
    if (Json::parseFromStream(builder, stream, &root, &errs) && root.isObject()) {
        if (root.isMember("description") && root["description"].isString()) {
            description = root["description"].asString();
        }
        if (root.isMember("suggested_name") && root["suggested_name"].isString()) {
            suggested_name = root["suggested_name"].asString();
        } else if (root.isMember("suggested_filename") && root["suggested_filename"].isString()) {
            suggested_name = root["suggested_filename"].asString();
        } else if (root.isMember("filename") && root["filename"].isString()) {
            suggested_name = root["filename"].asString();
        }
    }

    // Fallback: If no structured JSON, use raw text as description
    if (description.empty()) {
        description = raw_content;
    }

    return {description, sanitize_suggested_filename(suggested_name)};
}

} // namespace

EndpointImageAnalyzer::EndpointImageAnalyzer(std::shared_ptr<IAIProvider> provider)
    : provider_(std::move(provider))
{
    if (!provider_) {
        throw std::invalid_argument("EndpointImageAnalyzer requires a non-null IAIProvider instance.");
    }
}

ImageAnalysisResult EndpointImageAnalyzer::analyze(const std::filesystem::path& image_path)
{
    const auto start_time = std::chrono::steady_clock::now();
    ImageAnalysisResult result;

    const QString qpath = QString::fromStdString(Utils::path_to_utf8(image_path));
    std::string decode_error;
    QImage decoded = ImageDecodeUtils::decode_image_with_webp_fallback(
        qpath,
        QSize(kMaxImageDimension, kMaxImageDimension),
        &decode_error
    );

    std::vector<uint8_t> image_bytes;
    std::string mime_type = "image/jpeg";

    if (!decoded.isNull()) {
        QByteArray buffer;
        QBuffer qbuf(&buffer);
        if (qbuf.open(QIODevice::WriteOnly)) {
            if (decoded.hasAlphaChannel()) {
                decoded.save(&qbuf, "PNG");
                mime_type = "image/png";
            } else {
                decoded.save(&qbuf, "JPEG", kJpegQuality);
                mime_type = "image/jpeg";
            }
            image_bytes.assign(
                reinterpret_cast<const uint8_t*>(buffer.constData()),
                reinterpret_cast<const uint8_t*>(buffer.constData()) + buffer.size()
            );
        }
    }

    // Fallback: Direct binary read if decoding returned null but file exists
    if (image_bytes.empty()) {
        std::ifstream file(image_path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            const auto size = file.tellg();
            if (size > 0 && size <= 20 * 1024 * 1024) { // Max 20MB raw file
                file.seekg(0, std::ios::beg);
                image_bytes.resize(static_cast<size_t>(size));
                file.read(reinterpret_cast<char*>(image_bytes.data()), size);
                const std::string ext = image_path.extension().string();
                if (ext == ".png") mime_type = "image/png";
                else if (ext == ".webp") mime_type = "image/webp";
                else mime_type = "image/jpeg";
            }
        }
    }

    if (image_bytes.empty()) {
        throw std::runtime_error("Failed to load or decode image: " +
                                 Utils::path_to_utf8(image_path) +
                                 (decode_error.empty() ? "" : " (" + decode_error + ")"));
    }

    const auto decode_duration = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start_time
    ).count();

    // Construct canonical AIRequest with multimodal content part
    AIRequest req;
    req.add_system_message(
        "You are an image analysis assistant for a file organizer. "
        "Analyze the provided image and respond with a single valid JSON object containing exactly two keys: "
        "\"description\" (1-2 clear sentences describing the main subject, setting, and details) and "
        "\"suggested_name\" (a concise, descriptive file name in lower snake_case or kebab-case without extension)."
    );

    req.add_user_multimodal_message(
        "Describe this image and suggest a concise filename for it.",
        std::move(image_bytes),
        mime_type
    );

    req.with_temperature(0.2f);
    req.with_response_format("json_object");

    const AIResponse resp = provider_->generate_response(req);

    if (resp.has_error()) {
        const std::string err_msg = resp.error_message.value_or("HTTP " + std::to_string(resp.http_status));
        if (resp.http_status == 400 || err_msg.find("image") != std::string::npos ||
            err_msg.find("vision") != std::string::npos || err_msg.find("multimodal") != std::string::npos) {
            throw std::runtime_error("The selected AI model does not support image input: " + err_msg);
        }
        throw std::runtime_error("Endpoint visual analysis failed: " + err_msg);
    }

    const auto [description, suggested_name] = parse_vision_response(resp.text());
    result.description = description;
    result.suggested_name = suggested_name;

    const auto total_duration = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start_time
    ).count();

    result.diagnostics.available = true;
    result.diagnostics.bitmap_load_ms = decode_duration;
    result.diagnostics.total_ms = total_duration;

    return result;
}
