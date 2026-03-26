### Requirement: TSV Parser Initialisation
The TSV module SHALL initialise a streaming parser over a caller-provided mutable buffer without allocating memory.

#### Scenario: Successful initialisation
- **WHEN** `fun_tsv_init(FunTsvState *state, char *data)` is called with a non-null state and non-null null-terminated mutable buffer
- **THEN** the parser is ready to yield rows from the buffer
- **AND** the buffer will be modified in-place during parsing (tabs and newlines replaced with null bytes)

#### Scenario: Null arguments
- **WHEN** `fun_tsv_init` is called with a null state or null data pointer
- **THEN** an error result is returned

### Requirement: TSV Row Iteration
The TSV module SHALL yield one row at a time from the parsed buffer with fields accessible by index.

#### Scenario: Advance to next row
- **WHEN** `fun_tsv_next(FunTsvState *state, FunTsvRow *row)` is called after a successful init
- **THEN** `row->fields[i]` points to the i-th tab-separated field in the current row
- **AND** `row->count` contains the number of fields in the row
- **AND** the return value is true

#### Scenario: End of data
- **WHEN** `fun_tsv_next` is called and no more rows remain
- **THEN** the return value is false

#### Scenario: Empty rows skipped
- **WHEN** the buffer contains consecutive newlines
- **THEN** empty rows are skipped and the next non-empty row is returned

#### Scenario: Row with extra columns
- **WHEN** a row contains more tab-separated fields than the minimum expected
- **THEN** all fields up to FUN_TSV_MAX_FIELDS are accessible via `row->fields`

### Requirement: TSV Field Lifetime
The TSV module SHALL guarantee field pointer validity for the lifetime of the input buffer.

#### Scenario: Fields valid after iteration
- **WHEN** `fun_tsv_next` has been called and a row has been populated
- **THEN** `row->fields[i]` pointers remain valid as long as the original buffer is not freed or overwritten
- **AND** copying the pointer is sufficient to retain the field value

### Requirement: TSV State Stack-Allocatable
The TSV module SHALL require no heap allocation; state SHALL fit on the stack.

#### Scenario: Stack allocation
- **WHEN** a caller declares `FunTsvState state` as a local variable
- **THEN** no calls to `fun_memory_allocate` are required before calling `fun_tsv_init`
