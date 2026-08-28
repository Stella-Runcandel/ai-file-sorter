#ifndef LLMCLIENT_HPP
#define LLMCLIENT_HPP

#include "ILLMClient.hpp"
#include "IAIProvider.hpp"
#include <Types.hpp>
#include <memory>
#include <string>

/**
 * @brief LLMClient adapting the canonical IAIProvider (OpenAICompatibleProvider)
 * to the application's legacy ILLMClient interface.
 *
 * Translates CategorizationService and AnalysisCoordinator file categorization
 * and JSON completion requests into canonical AIRequest domain models and delegates
 * execution directly to IAIProvider.
 */
class LLMClient : public ILLMClient {
public:
    /**
     * @brief Create an OpenAI-compatible client adapter, optionally targeting a custom base URL.
     */
    LLMClient(std::string api_key, std::string model, std::string base_url = std::string());
    ~LLMClient() override;

    std::string categorize_file(const std::string& file_name,
                                const std::string& file_path,
                                FileType file_type,
                                const std::string& consistency_context) override;
    std::string complete_prompt(const std::string& prompt,
                                int max_tokens) override;
    void set_prompt_logging_enabled(bool enabled) override;

    /**
     * @brief Direct access to underlying IAIProvider.
     */
    IAIProvider* provider() const { return provider_.get(); }

private:
    std::string effective_model() const;
    std::string resolve_api_url() const;

    std::string api_key_;
    std::string model_;
    std::string base_url_;
    bool prompt_logging_enabled_{false};
    std::string last_prompt_;
    std::unique_ptr<IAIProvider> provider_;
};

#endif
