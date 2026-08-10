## ADDED Requirements

### Requirement: GPT-OSS-20B demo application
The system SHALL provide a CLI demo at `demos/gpt-demo/` that loads a GPT-OSS-20B GGUF model and performs single-request inference.

#### Scenario: Command-line prompt
- **WHEN** `demo.exe "Hello how are you?"` is executed
- **THEN** the demo prints a model-generated response to stdout

#### Scenario: Timing output
- **WHEN** the inference completes
- **THEN** the demo prints evaluation time in seconds and tokens per second

#### Scenario: Missing model file
- **WHEN** the GGUF model file is not found at the expected path
- **THEN** the demo prints an error message and exits with non-zero code
