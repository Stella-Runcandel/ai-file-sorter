# Configuration and Environment

This page collects the configuration surfaces that are most useful for
contributors, packagers, and integrators. The main README can stay focused on
normal setup and usage.

## Local settings and storage

- `config.ini` lives under the app config directory.
- `AI_FILE_SORTER_CONFIG_DIR` overrides the base config directory.
- `CATEGORIZATION_CACHE_FILE` overrides the categorization SQLite filename
  inside the config directory.

## Timeouts, pacing, and logs

- `AI_FILE_SORTER_REMOTE_LLM_TIMEOUT` - Timeout in seconds for OpenAI/Gemini API calls.
- `AI_FILE_SORTER_CUSTOM_LLM_TIMEOUT` - Timeout in seconds for custom OpenAI-compatible endpoints.
- `AI_FILE_SORTER_REMOTE_REQUESTS_PER_MINUTE` - Client-side pacing limit for rate-limited endpoints.

These knobs are most useful when diagnosing slow or rate-limited API providers.

## Headless setting overlays

Headless callers can pass `--settings-overrides-file <json-file>` to inject a
non-persistent settings overlay for one run. This is the preferred way to steer
integration-specific behavior without rewriting the user's saved settings.

## Windows naming and migration note

New registry/settings/integration paths should use `HFStudio`. Compatibility
code may still need to read or clean legacy `Quicknode` locations during
migration or uninstall flows.

## Related references

- [Headless runtime contract](headless-runtime-contract.md)
- [Updater contract](updater-contract.md)
- [Windows release builds](windows-release-builds.md)
