## ADDED Requirements

### Requirement: sin implementation
The system SHALL implement `fun_math_sin` using range reduction modulo 2π followed by a Taylor polynomial, achieving relative error below 1e-3 for inputs with absolute value ≤ 50.

#### Scenario: sin of values in valid range
- **WHEN** sin is called with inputs in [-50, 50]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: sin of zero
- **WHEN** sin is called with 0.0
- **THEN** it returns 0.0

#### Scenario: sin of NaN
- **WHEN** sin is called with NaN
- **THEN** it returns NaN

#### Scenario: sin odd symmetry
- **WHEN** sin is called with x
- **THEN** sin(-x) equals -sin(x) for any x in the valid domain

### Requirement: cos implementation
The system SHALL implement `fun_math_cos` using the identity `cos(x) = sin(x + π/2)`, achieving relative error below 1e-3 for inputs with absolute value ≤ 50.

#### Scenario: cos of values in valid range
- **WHEN** cos is called with inputs in [-50, 50]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: cos of zero
- **WHEN** cos is called with 0.0
- **THEN** it returns 1.0

#### Scenario: cos of NaN
- **WHEN** cos is called with NaN
- **THEN** it returns NaN

### Requirement: tanh implementation
The system SHALL implement `fun_math_tanh` using `fun_math_exp`, with saturation clamping at ±1 for |x| ≥ 10, achieving relative error below 1e-3 for all real inputs.

#### Scenario: tanh normal range
- **WHEN** tanh is called with inputs in [-10, 10]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: tanh saturation
- **WHEN** tanh is called with inputs ≥ 10
- **THEN** it returns 1.0
- **WHEN** tanh is called with inputs ≤ -10
- **THEN** it returns -1.0

#### Scenario: tanh special values
- **WHEN** tanh is called with 0.0
- **THEN** it returns 0.0
- **WHEN** tanh is called with +inf
- **THEN** it returns 1.0
- **WHEN** tanh is called with -inf
- **THEN** it returns -1.0
- **WHEN** tanh is called with NaN
- **THEN** it returns NaN

### Requirement: sigmoid implementation
The system SHALL implement `fun_math_sigmoid` using `fun_math_exp`, with saturation clamping at 0 and 1 for |x| ≥ 20, achieving relative error below 1e-3 for all real inputs.

#### Scenario: sigmoid normal range
- **WHEN** sigmoid is called with inputs in [-20, 20]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: sigmoid saturation
- **WHEN** sigmoid is called with inputs ≥ 20
- **THEN** it returns 1.0
- **WHEN** sigmoid is called with inputs ≤ -20
- **THEN** it returns 0.0

#### Scenario: sigmoid special values
- **WHEN** sigmoid is called with 0.0
- **THEN** it returns 0.5
- **WHEN** sigmoid is called with +inf
- **THEN** it returns 1.0
- **WHEN** sigmoid is called with -inf
- **THEN** it returns 0.0
- **WHEN** sigmoid is called with NaN
- **THEN** it returns NaN

### Requirement: silu implementation
The system SHALL implement `fun_math_silu` using `fun_math_sigmoid`, achieving relative error below 1e-3 for all real inputs.

#### Scenario: silu normal range
- **WHEN** silu is called with inputs in [-20, 20]
- **THEN** the result differs from the IEEE 754 reference by less than 1e-3 relative error

#### Scenario: silu saturation
- **WHEN** silu is called with inputs ≥ 20
- **THEN** it returns x (approximately the identity function)
- **WHEN** silu is called with inputs ≤ -20
- **THEN** it returns 0.0

#### Scenario: silu special values
- **WHEN** silu is called with 0.0
- **THEN** it returns 0.0
- **WHEN** silu is called with +inf
- **THEN** it returns +inf
- **WHEN** silu is called with -inf
- **THEN** it returns 0.0
- **WHEN** silu is called with NaN
- **THEN** it returns NaN

## MODIFIED Requirements

### Requirement: Unimplemented functions return sentinel
Functions deferred to future chunks SHALL return 0.0 so that tests can compile and run, making failures clearly attributable to missing implementations.

#### Scenario: Deferred scalar functions
- **WHEN** `fun_math_sin`, `fun_math_cos`, `fun_math_tanh`, `fun_math_sigmoid`, or `fun_math_silu` is called
- **THEN** it returns 0.0

#### Scenario: Deferred vector functions
- **WHEN** `fun_math_silu_f32`, `fun_math_rms_norm_f32`, `fun_math_swiglu_f32`, or `fun_math_softmax_f32` is called
- **THEN** it returns 0.0 or returns immediately for void functions
