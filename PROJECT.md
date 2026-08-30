# Project: Endpoint-Based Multimodal AI Architecture (Stella-Runcandel/ai-file-sorter, upstream: hyperfield/ai-file-sorter)

## Architecture
Modernize the application from an in-process llama.cpp / mtmd coupling to an endpoint-first, provider-agnostic AI architecture.
The system supports both text-only and multimodal (image + text) categorization via standard OpenAI-compatible endpoints (`/v1/chat/completions`) with strict ZERO silent fallback.

```
┌──────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                 CANONICAL AI DOMAIN LAYER (R1)                                   │
│  • AIContentPart (Text, Binary Image) • AIMessage (Role, Parts) • AIRequest • AIResponse         │
│  • ProviderCapabilities (status, latency, models, vision support, JSON mode)                     │
└─────────────────────────────────┬───────────────────────────────┬────────────────────────────────┘
                                  │                               │
        ┌─────────────────────────┴──────────────┐   ┌────────────┴────────────────────────┐
        ▼                                        ▼   ▼                                     ▼
┌────────────────────────────────────────┐ ┌───────────────────────────────────────────────────────┐
│     HTTP PROVIDER & RESOLVER (R2)      │ │          MULTIMODAL PREPROCESSING (R3)                │
│ • IAIProvider Interface                │ │ • Client-Side Downscaling (Max 2048x2048)             │
│ • OpenAICompatibleProvider (libcurl)   │ │ • In-Memory JPEG (Q=85) & PNG Transcoding             │
│ • EndpointUrlResolver (Ollama/LM Studio│ │ • Base64 Data URI ("data:image/jpeg;base64,...")      │
│ • /v1/models Probe & Capability Audit  │ │ • OpenAI Multimodal Message Schema                    │
└────────────────────────────────────────┘ └───────────────────────────────────────────────────────┘
                                  │                               │
                                  └──────────────┬────────────────┘
                                                 ▼
┌──────────────────────────────────────────────────────────────────────────────────────────────────┐
│                    DUAL-PATH INTEGRATION & ZERO-FALLBACK ROUTING (R4)                            │
│ • CategorizationService & AnalysisCoordinator Integration                                        │
│ • User Mode: Remote Endpoint (IAIProvider) ──► On Failure: Explicit Error (Zero Fallback)       │
│ • User Mode: Legacy Local LLM (LocalLLMClient / Llava) ──► Intact Legacy Execution               │
└────────────────────────────────────────────────┬─────────────────────────────────────────────────┘
                                                 ▼
┌──────────────────────────────────────────────────────────────────────────────────────────────────┐
│                             SAFE LOGGING & SETTINGS UI (R5)                                      │
│ • SecretMasker: Masks API Keys (sk-***cdef), Base64 URI strings, and Auth Headers                │
│ • CustomApiDialog & LLMSelectionDialog: URL input, model auto-discovery, live connection test   │
└──────────────────────────────────────────────────────────────────────────────────────────────────┘
```

## Feature Inventory
| # | Feature | Description | Milestone | Source |
|---|---------|-------------|-----------|--------|
| 1 | Canonical Domain Models | `AIContentPart`, `AIMessage`, `AIRequest`, `AIResponse`, `ProviderCapabilities` decoupled from JSON | M1 | Survey R1 |
| 2 | Pure Provider Interface | `IAIProvider` abstract interface with sync/async completion, probe, and model discovery | M2 | Survey R2 |
| 3 | Smart Endpoint URL Resolver | `EndpointUrlResolver` normalizing Ollama (:11434), LM Studio (:1234/v1), llama-server (:8080), Cloud APIs | M2 | Survey R2 |
| 4 | Generic OpenAI HTTP Provider | `OpenAICompatibleProvider` with libcurl, Bearer auth, custom timeouts, and HTTP status handling | M2 | Survey R2 |
| 5 | Model Discovery & Probe | Dynamic discovery via `/v1/models` and capability probing (latency, status, vision heuristic) | M2 | Survey R2 |
| 6 | Client-Side Image Downscaler | Aspect-preserving downscaling to max 2048x2048 in memory | M3 | Survey R3 |
| 7 | In-Memory Transcoder & Base64 | JPEG (Q=85)/PNG in-memory compression and `data:image/jpeg;base64,...` data URI construction | M3 | Survey R3 |
| 8 | OpenAI Multimodal Schema | Standard message formatting `{"type": "image_url", "image_url": {"url": "..."}}` | M3 | Survey R3 |
| 9 | Vision Capability Guard | Preflight check returning graceful error `"The selected AI model does not support image input."` | M3 | Survey R3 |
| 10| Dual-Path Adapter | `AIProviderLLMClientAdapter` adapting `IAIProvider` to `ILLMClient` for seamless integration | M4 | Survey R4 |
| 11| Visual Pipeline Routing | `AnalysisCoordinator` routing images through endpoint provider without calling `LlavaImageAnalyzer` | M4 | Survey R4 |
| 12| Strict Zero Fallback | Endpoint errors reported directly with zero silent invocation of embedded `llama.cpp` or local GPU/CPU | M4 | Survey R4 |
| 13| Secret & Payload Masking | `SecretMasker` redacting API keys, raw base64 payloads, and Auth headers from logs | M5 | Survey R5 |
| 14| Settings UI Controls | `CustomApiDialog` & `LLMSelectionDialog` with URL normalization, model selector, discovery | M5 | Survey R5 |
| 15| Live Connection Testing | Async connection test button with latency badge, HTTP status badge, and vision capability badge | M5 | Survey R5 |
| 16| Deterministic Mock Server | `MockOpenAIServer` loopback HTTP harness simulating Ollama, LM Studio, rate limits, and errors | E2E Track | Survey Testing |
| 17| 4-Tier Automated Test Suite | Tiers 1-4 test suite covering features, boundaries, combinations, and real-world workloads | E2E Track | Survey Testing |
| 18| 100% E2E Pass & Hardening | Phase 1: 100% pass of E2E suite; Phase 2: Adversarial coverage hardening (Tier 5) | Final M6 | Survey Final |

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Canonical AI Domain Model (R1) | Pure C++20 domain abstractions (`AIContentPart`, `AIMessage`, `AIRequest`, `AIResponse`, `ProviderCapabilities`) | none | DONE |
| M2 | OpenAI HTTP Provider & URL Resolver (R2) | `IAIProvider`, `EndpointUrlResolver`, `OpenAICompatibleProvider`, `/v1/models` probe, Bearer auth | M1 | DONE |
| M3 | Multimodal Vision & Preprocessing (R3) | `ImagePreprocessingService`, 2048x2048 downscaler, JPEG encoder, base64 URI, vision capability guard | M1 | DONE |
| M4 | Dual-Path Integration & Zero Fallback (R4) | `AIProviderLLMClientAdapter`, `CategorizationService` & `AnalysisCoordinator` routing, zero fallback | M2, M3 | DONE |
| M5 | Settings UI & Safe Logging (R5) | `SecretMasker`, `CustomApiDialog`, `LLMSelectionDialog`, connection test & capability badges | M2, M4 | DONE |
| E2E | E2E Testing Track | `MockOpenAIServer` test harness, 4-tier Catch2 test suite (Tiers 1-4), publish `TEST_READY.md` | M1 (runs parallel) | DONE |
| M6 | Final Integration & Hardening | Phase 1: Pass 100% E2E tests (Tiers 1-4); Phase 2: Adversarial Hardening (Tier 5) | M5, E2E | DONE |

## Interface Contracts

### Canonical Domain Model (`app/include/AIContentPart.hpp`, `AIMessage.hpp`, `AIRequest.hpp`, `AIResponse.hpp`, `ProviderCapabilities.hpp`)
- `AIContentPart::from_text(std::string text) -> AIContentPart`
- `AIContentPart::from_image(std::vector<uint8_t> bytes, std::string mime_type = "image/jpeg", std::optional<std::string> detail = "auto") -> AIContentPart`
- `AIMessage { Role role; std::vector<AIContentPart> parts; }`
- `AIRequest { std::string model; std::vector<AIMessage> messages; std::optional<float> temperature; std::optional<int> max_tokens; std::optional<std::string> response_format; std::chrono::milliseconds timeout; }`
- `AIResponse { bool success; std::string content; std::string model; Usage usage; std::string finish_reason; long http_status; double latency_ms; std::optional<std::string> error_message; }`
- `ProviderCapabilities { bool endpoint_reachable; long latency_ms; bool supports_chat_completions; CapabilityStatus vision_capability; bool supports_json_mode; bool supports_model_discovery; std::vector<std::string> available_models; std::string detected_server_flavor; }`

### Generic Provider Interface (`app/include/IAIProvider.hpp`)
- `virtual AIResponse complete(const AIRequest& request) = 0;`
- `virtual std::future<AIResponse> complete_async(const AIRequest& request) = 0;`
- `virtual ProviderCapabilities probe_capabilities() = 0;`
- `virtual std::vector<std::string> discover_models() = 0;`
- `virtual void set_logging_enabled(bool enabled) = 0;`

### URL Normalizer (`app/include/EndpointUrlResolver.hpp`)
- `static std::string resolve_chat_completions_url(std::string_view input_url);`
- `static std::string resolve_models_url(std::string_view input_url);`

### Vision Preprocessing (`app/include/ImagePreprocessingService.hpp`)
- `static AIContentPart load_and_preprocess(const std::filesystem::path& image_path);`
- `static std::string to_data_uri(const AIContentPart::ImageData& image_data);`

### Safe Logging Redaction (`app/include/SecretMasker.hpp`)
- `static std::string mask_api_key(std::string_view api_key);`
- `static std::string sanitize_payload_for_logging(std::string_view payload);`
- `static std::string sanitize_http_headers(std::string_view header_line);`

## Code Layout
- Headers: `app/include/`
  - `AIContentPart.hpp`, `AIMessage.hpp`, `AIRequest.hpp`, `AIResponse.hpp`, `ProviderCapabilities.hpp`
  - `IAIProvider.hpp`, `EndpointUrlResolver.hpp`, `OpenAICompatibleProvider.hpp`
  - `ImagePreprocessingService.hpp`, `AIProviderLLMClientAdapter.hpp`, `SecretMasker.hpp`
- Implementation: `app/lib/`
  - `EndpointUrlResolver.cpp`, `OpenAICompatibleProvider.cpp`, `ImagePreprocessingService.cpp`
  - `AIProviderLLMClientAdapter.cpp`, `SecretMasker.cpp`
  - `AnalysisCoordinator.cpp`, `CategorizationService.cpp`, `CustomApiDialog.cpp`, `LLMSelectionDialog.cpp`
- Test Harness & Tests: `tests/`
  - `tests/helpers/MockHttpServer.hpp`, `tests/helpers/MockHttpServer.cpp`
  - `tests/unit/test_ai_domain_models.cpp`
  - `tests/unit/test_endpoint_url_resolver.cpp`
  - `tests/unit/test_openai_compatible_provider.cpp`
  - `tests/unit/test_vision_image_preprocessor.cpp`
  - `tests/unit/test_safe_logging_secret_masking.cpp`
  - `tests/unit/test_dual_path_coexistence.cpp`
  - `tests/unit/test_e2e_mixed_workload.cpp`
