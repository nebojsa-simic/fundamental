# gguf Specification

## Purpose
TBD - created by archiving change add-gguf-module. Update Purpose after archive.
## Requirements
### Requirement: GGUF file open and close
The system SHALL provide `fun_gguf_open(String path)` to memory-map a GGUF file and `fun_gguf_close(GGufFile *file)` to release resources.

#### Scenario: Open valid GGUF file
- **WHEN** `fun_gguf_open` is called with a valid GGUF v3 file path
- **THEN** it returns a non-NULL GGufFile pointer

#### Scenario: Open invalid file
- **WHEN** `fun_gguf_open` is called with a non-GGUF file
- **THEN** it returns an error

#### Scenario: Close releases mmap
- **WHEN** `fun_gguf_close` is called on an open file
- **THEN** the memory mapping and file handles are released

### Requirement: GGUF metadata query
The system SHALL provide functions to query metadata key-value pairs by key name: string, int32, uint32, and float32 types.

#### Scenario: Get metadata string
- **WHEN** `fun_gguf_get_metadata_string` is called with an existing key
- **THEN** it returns the string value

#### Scenario: Get metadata integer
- **WHEN** `fun_gguf_get_metadata_u32` is called with key "gpt-oss.block_count"
- **THEN** it returns 24

#### Scenario: Missing metadata key
- **WHEN** a metadata query is made for a key not present in the file
- **THEN** it returns an error

### Requirement: GGUF tensor info query
The system SHALL provide functions to query tensor metadata by tensor name: byte offset, byte size, and GGUF type.

#### Scenario: Get tensor offset
- **WHEN** `fun_gguf_get_tensor_offset` is called with a tensor name present in the file
- **THEN** it returns the byte offset of the tensor data in the file

#### Scenario: Get tensor size
- **WHEN** `fun_gguf_get_tensor_size` is called with a tensor name
- **THEN** it returns the size of the tensor in bytes

#### Scenario: Get tensor type
- **WHEN** `fun_gguf_get_tensor_type` is called with an F32 tensor name
- **THEN** it returns 0 (GGUF_TYPE_F32)

### Requirement: Q8_0 dequantization
The system SHALL dequantize Q8_0-encoded tensors to float32 arrays.

#### Scenario: Dequant Q8_0 tensor
- **WHEN** `fun_gguf_dequant_q8_0` is called with a Q8_0 tensor name and a float32 output buffer
- **THEN** the buffer contains the tensor data converted to float32, with each block of 32 values decompressed using its block scale

### Requirement: MXFP4 dequantization
The system SHALL dequantize MXFP4-encoded tensors to float32 arrays.

#### Scenario: Dequant MXFP4 tensor
- **WHEN** `fun_gguf_dequant_mxfp4` is called with an MXFP4 tensor name and a float32 output buffer
- **THEN** the buffer contains unpacked float32 values, each decoded from a 4-bit sign-exponent-mantissa representation scaled by a per-block E8M0 exponent

