#pragma once

#ifdef AI_FILE_SORTER_TEST_BUILD

#if AI_FILE_SORTER_ENABLE_EMBEDDED_AI
#include "LocalLLMClient.hpp"
#include "llama.h"
#endif

#include "Types.hpp"
#include "LocalLLMResponseSanitizer.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace LocalLLMTestAccess {

#if AI_FILE_SORTER_ENABLE_EMBEDDED_AI
enum class BackendPreference {
    Auto,
    Cpu,
    Cuda,
    Vulkan
};

BackendPreference detect_preferred_backend();
bool apply_cpu_backend(llama_model_params& params, BackendPreference preference);
bool apply_vulkan_backend(const std::string& model_path,
                          llama_model_params& params);
bool handle_cuda_forced_off(bool cuda_forced_off,
                            BackendPreference preference,
                            llama_model_params& params);
bool configure_cuda_backend(const std::string& model_path,
                            llama_model_params& params);
/**
 * @brief Prepared model parameters plus any pre-load status emitted during backend selection.
 */
struct PreparedModelParamsResult {
    llama_model_params params;
    std::optional<LocalLLMClient::Status> status;
};
/**
 * @brief Builds model parameters and captures preflight backend status for tests.
 * @param model_path Path to the GGUF model used for backend preparation.
 * @return Prepared parameters together with any emitted status.
 */
PreparedModelParamsResult prepare_model_params_result_for_testing(const std::string& model_path);
llama_model_params prepare_model_params_for_testing(const std::string& model_path);
/**
 * @brief Builds the GPU-layer retry sequence used after an optimistic load failure.
 * @param optimistic_layers First load attempt, typically the larger optimistic estimate.
 * @param conservative_layers Second load attempt, typically the smaller conservative estimate.
 * @return Strict retry ladder with duplicates removed and each later value reduced geometrically.
 */
std::vector<int> gpu_layer_retry_candidates_for_testing(int optimistic_layers,
                                                        int conservative_layers);
std::string categorization_system_prompt_for_testing(const std::string& file_path,
                                                     FileType file_type,
                                                     std::string_view consistency_context = {});
std::string categorization_user_prompt_for_testing(const std::string& file_name,
                                                   const std::string& file_path,
                                                   FileType file_type,
                                                   const std::string& consistency_context);
#endif

inline std::string sanitize_output_for_testing(const std::string& output) {
    return LocalLLMResponseSanitizer::sanitize_categorization_output(output);
}

} // namespace LocalLLMTestAccess

#endif // AI_FILE_SORTER_TEST_BUILD
