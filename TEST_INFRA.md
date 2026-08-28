# E2E Test Infra: Phase 1 Endpoint-Based Multimodal AI Architecture

## Test Philosophy
- Opaque-box, requirement-driven testing derived from `ORIGINAL_REQUEST.md`.
- Zero external network dependencies: 100% deterministic offline execution via in-process `MockOpenAIServer` loopback socket harness.
- Methodology: Category-Partition + Boundary Value Analysis (BVA) + Pairwise Combinations + Real-World Workload Testing.

## Feature Inventory
| # | Feature | Source (Requirement) | Tier 1 | Tier 2 | Tier 3 |
|---|---------|---------------------|:------:|:------:|:------:|
| 1 | Canonical Domain Models (`AIRequest`, `AIResponse`, `AIContentPart`) | ORIGINAL_REQUEST §R1 | 5 | 5 | ✓ |
| 2 | Endpoint URL Resolver (`EndpointUrlResolver`) | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ |
| 3 | OpenAI-Compatible HTTP Provider (`OpenAICompatibleProvider`) | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ |
| 4 | Vision Preprocessor & Base64 Encoder (`ImagePreprocessingService`) | ORIGINAL_REQUEST §R3 | 5 | 5 | ✓ |
| 5 | Dual-Path Integration & Zero Silent Fallback (`CategorizationService`) | ORIGINAL_REQUEST §R4 | 5 | 5 | ✓ |
| 6 | Secret & Base64 Log Redaction (`SecretMasker`) | ORIGINAL_REQUEST §R5 | 5 | 5 | ✓ |

## Test Architecture
- Test Runner: Catch2 v3 (`Catch2::Catch2WithMain`) linked to `ai_file_sorter_tests` target.
- Invocation: `ctest --test-dir build-tests --output-on-failure` or `./build-tests/tests/Release/ai_file_sorter_tests "[tier1]"`.
- Mock Server: `MockOpenAIServer` (`tests/helpers/MockHttpServer.hpp`) using `QTcpServer` listening on `127.0.0.1:0`.
  - Simulates: Ollama, LM Studio, llama-server, OpenAI Vision, HTTP 429 (Retry-After), HTTP 400 vision rejection, slow responses, truncated JSON, TCP resets.
- Directory Layout:
  - `tests/helpers/`: `MockHttpServer.hpp`, `MockHttpServer.cpp`, `TestHelpers.hpp`
  - `tests/unit/`:
    - `test_ai_domain_models.cpp` (Tier 1 & Tier 2)
    - `test_endpoint_url_resolver.cpp` (Tier 1 & Tier 2)
    - `test_openai_compatible_provider.cpp` (Tier 1, Tier 2, Tier 3)
    - `test_vision_image_preprocessor.cpp` (Tier 1 & Tier 2)
    - `test_safe_logging_secret_masking.cpp` (Tier 3)
    - `test_dual_path_coexistence.cpp` (Tier 3)
    - `test_e2e_mixed_workload.cpp` (Tier 4)

## Real-World Application Scenarios (Tier 4)
| # | Scenario | Features Exercised | Complexity |
|---|----------|--------------------|------------|
| 1 | Full 12-Stage Headless Mixed-Folder Categorization | PDF/DOCX extractors, Image downscaler, Base64 URI, Audio/Media ID3, Protected Git skip, SQLite commit, Undo manifest generation & move rollback | High |
| 2 | Pure Vision Categorization with Mock Ollama LLaVA | Image downscaling, JPEG encoding, Base64 URI, Ollama /v1/chat/completions schema, SQLite cache | Medium |
| 3 | Text-Only Model Vision Graceful Failure Handling | Non-vision model selection, image input rejection with clear error, zero fallback to local llama.cpp | Medium |
| 4 | Offline Local Server Switch & Settings Reload | Settings load/save, URL normalization, live probe, model auto-discovery dropdown | Medium |
| 5 | Large Batch Move with Rollback Verification | Multi-file categorizations, batch file moves, atomic undo manifest inversion validation | High |

## Coverage Thresholds
- Tier 1: ≥5 test cases per feature (Total ≥ 30 cases)
- Tier 2: ≥5 boundary & corner test cases per feature (Total ≥ 30 cases)
- Tier 3: Pairwise coverage for provider switching, secret masking, and coexistence without fallback (Total ≥ 5 cases)
- Tier 4: Realistic multi-format application workloads (Total ≥ 5 cases)
- **Total Minimum Target: ≥ 70 comprehensive test cases**
