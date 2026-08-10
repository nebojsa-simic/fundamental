# gpt-demo Specification

## Purpose
TBD - created by archiving change gpt-demo. Update Purpose after archive.
## Requirements
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

### Requirement: Fused MXFP4 expert inference
The gpt-demo SHALL compute each selected MoE expert's gate/up/down projections
with `fun_math_matrix_vector_mxfp4_f32` directly over the memory-mapped GGUF
tensor data, and SHALL NOT dequantize expert weights into f32 heap buffers,
materialize per-expert cache files, or look up tensor names in the forward
pass.

#### Scenario: Per-layer tensor bases precomputed
- **WHEN** a layer is first loaded
- **THEN** the demo stores the GGUF offsets of the gate/up/down MXFP4 weight
  tensors and their f32 bias tensors, plus the byte stride per expert, and the
  forward pass indexes into them without name lookups

#### Scenario: Expert projections use the fused primitive
- **WHEN** expert `e` in layer `l` is selected
- **THEN** the gate and up projections are computed with
  `fun_math_matrix_vector_mxfp4_f32` against the hidden state and the down
  projection against the intermediate activation, each with the expert's bias
  vector

#### Scenario: No persistent cache
- **WHEN** generation runs
- **THEN** no expert weight files are read or written and no cache state is
  consulted; each expert's weights are read from the GGUF mapping on use

#### Scenario: Greedy output preserved
- **WHEN** generation runs after the refactor with the same prompt and model
- **THEN** the greedy argmax token sequence is unchanged from the pre-refactor
  baseline (no quantization drift)

### Requirement: Cross-platform demo build
The gpt-demo SHALL build on both Windows and Linux from platform-specific
build scripts that select platform-specific arch files, and SHALL contain no
operating-system-specific code in its shared source files.

#### Scenario: Windows build
- **WHEN** `build-windows-amd64.bat` is executed
- **THEN** it compiles the demo with the windows-amd64 timing arch file into
  `demo.exe`

#### Scenario: Linux build
- **WHEN** `build-linux-amd64.sh` is executed
- **THEN** it compiles the demo with the linux-amd64 timing arch file into a
  Linux executable using `gcc`

#### Scenario: Portable timing
- **WHEN** the demo measures elapsed time
- **THEN** it uses the platform's monotonic clock through a demo-local timing
  abstraction, not a direct OS API call in shared code

