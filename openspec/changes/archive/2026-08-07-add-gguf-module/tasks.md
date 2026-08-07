## 1. Public API Header

- [x] 1.1 Create `include/fundamental/gguf/gguf.h` with GGufFile type, open/close, metadata query, tensor info query, and dequant function declarations

## 2. Core Implementation

- [x] 2.1 Create `src/gguf/gguf.c` — header parsing (magic, version, tensor/kv counts), KV scan for metadata queries, tensor info scan for offset/size/type queries
- [x] 2.2 Create `src/gguf/gguf_dequant.c` — Q8_0 dequant (half-float scale conversion + int8 multiply), MXFP4 dequant (E8M0 scale + 4-bit unpack + float reconstruction)

## 3. Platform Layer

- [x] 3.1 Create `arch/gguf/windows-amd64/mmap.c` — CreateFileW, GetFileSizeEx, CreateFileMappingW, MapViewOfFile, UnmapViewOfFile, CloseHandle
- [x] 3.2 Create `arch/gguf/linux-amd64/mmap.c` — open, fstat, mmap, munmap, close

## 4. Tests

- [x] 4.1 Create `tests/gguf/test_data/tools/generate_minimal.c` — generates a minimal GGUF v3 file with 1 F32 tensor and 2 KV entries
- [x] 4.2 Create `tests/gguf/test_gguf.c` — verify open/close, metadata query, tensor offset/size/type, dequant
- [x] 4.3 Create `tests/gguf/build-windows-amd64.bat` and `tests/gguf/build-linux-amd64.sh`

## 5. Verification

- [x] 5.1 Build and run tests on Windows
- [x] 5.2 Build and run tests on Linux (Alpine WSL)
- [x] 5.3 Run code formatter
