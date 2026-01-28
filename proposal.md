# A Proposal to Add `<rational>` to the C++ Standard Library

|                   |                                                      |
|-------------------|------------------------------------------------------|
| **Document #:**   | DXXXXR0 (Placeholder - to be assigned by WG21)     |
| **Date:**         | 2026-01-28                                           |
| **Project:**      | Programming Language C++                             |
| **Audience:**     | Library Evolution Working Group (LEWG)               |
| **Reply-to:**     | Ali Can Demiralp                                     |

## Table of Contents

1. [Introduction](#1-introduction)
2. [Motivation and Scope](#2-motivation-and-scope)
3. [Impact on the Standard](#3-impact-on-the-standard)
4. [Design Decisions](#4-design-decisions)
5. [Technical Specification](#5-technical-specification)
6. [Implementation Experience](#6-implementation-experience)
7. [Proposed Wording](#7-proposed-wording)
8. [Acknowledgements](#8-acknowledgements)
9. [References](#9-references)

---

## 1. Introduction

This proposal introduces a new standard library component, `std::rational<T>`, which provides a type for representing rational numbers (fractions) with exact precision. The rational number type is a fundamental mathematical concept that enables exact arithmetic operations without the rounding errors inherent in floating-point representations.

Rational numbers are crucial in many domains including:
- Exact arithmetic and symbolic computation
- Music and audio processing (time signatures, tempo ratios)
- Computer graphics (aspect ratios, bezier curves)
- Financial calculations requiring exact precision
- Scientific computing where accumulated rounding errors are unacceptable

The proposed `std::rational<T>` template provides:
- Exact representation of rational numbers as a ratio of two integers
- Automatic reduction to canonical form (lowest terms)
- Comprehensive arithmetic operations
- Type-safe conversions between rational and arithmetic types
- Stream I/O support
- Constexpr support for compile-time computation

## 2. Motivation and Scope

### 2.1 Why Rational Numbers?

Floating-point arithmetic, while ubiquitous and highly optimized, suffers from inherent limitations:

```cpp
double x = 0.1 + 0.2;  // x != 0.3 due to rounding errors
```

For applications requiring exact arithmetic, rational numbers offer a superior alternative:

```cpp
std::rational r1{1, 10};  // 1/10
std::rational r2{2, 10};  // 2/10
auto r3 = r1 + r2;        // exactly 3/10
```

### 2.2 Prior Art

Several implementations demonstrate the utility of rational number types:

1. **Boost.Rational** - A mature implementation that has been widely used in production code for over two decades
2. **Julia's Rational** - Integral to Julia's numeric tower, demonstrating the value of built-in rational support
3. **Python's fractions.Fraction** - Part of Python's standard library since version 2.6
4. **GMP's mpq_t** - High-performance arbitrary-precision rational arithmetic

The existence of these implementations across multiple languages and libraries demonstrates a consistent need for rational number support.

### 2.3 Use Cases

#### Exact Computation
```cpp
// Financial calculations requiring exact precision
std::rational price_per_share{12345, 100};  // $123.45
std::rational shares{3};
auto total_cost = price_per_share * shares;  // Exact: 37035/100
```

#### Music and Audio
```cpp
// Musical time signatures and tempo calculations
std::rational time_signature{3, 4};    // 3/4 time
std::rational note_duration{1, 8};     // Eighth note
auto beats = time_signature / note_duration;  // 6 eighth notes per measure
```

#### Computer Graphics
```cpp
// Exact aspect ratio calculations
std::rational aspect_ratio{16, 9};
std::rational<int> width{1920};
auto height = width / aspect_ratio;  // Exact: 1080
```

### 2.4 Scope

This proposal focuses on:
- A template class `std::rational<T>` where `T` is an integral type
- Maintaining rational numbers in canonical form (lowest terms, positive denominator)
- Support for all standard arithmetic operations
- Conversion facilities to and from floating-point types
- Constexpr support for compile-time computation
- Stream I/O operators
- User-defined literals for convenience

This proposal does **not** include:
- Arbitrary-precision arithmetic (users can combine with `std::int_fast64_t` or third-party bignum libraries)
- Transcendental functions (these inherently require approximation)
- Automatic overflow detection (consistent with built-in integer behavior)

## 3. Impact on the Standard

### 3.1 Impact on the C++ Standard Library

This proposal is a pure library addition and requires no changes to the core language. It adds:
- A new header `<rational>`
- A new template class `std::rational<T>` and associated free functions
- User-defined literals in an inline namespace for opt-in usage

### 3.2 Dependencies

The proposed `<rational>` header depends on existing standard library components:
- `<compare>` - For three-way comparison support (C++20)
- `<numeric>` - For `std::gcd` 
- `<iostream>` - For stream operators
- `<limits>` - For numeric traits
- `<cmath>` - For `std::frexp`, `std::exp2` in floating-point conversions
- `<type_traits>` - For SFINAE and concepts
- `<stdexcept>` - For `std::domain_error`

### 3.3 Compatibility

This proposal:
- Is entirely backward compatible (pure addition)
- Does not affect existing code
- Can be implemented using C++20 features (concepts, three-way comparison)
- Could be adapted for C++17 with minor modifications (replace concepts with SFINAE)

## 4. Design Decisions

### 4.1 Template Parameter

The `rational` class is parameterized on an integral type `T`:

```cpp
template <std::integral T>
class rational;
```

**Rationale:** This allows users to choose the underlying integer type based on their needs:
- `std::rational<int>` for most common cases
- `std::rational<int64_t>` for larger range
- `std::rational<__int128>` for very large values (where available)
- Users can integrate with arbitrary-precision libraries

### 4.2 Canonical Form

Rational numbers are automatically maintained in canonical form:
1. Numerator and denominator are coprime (GCD = 1)
2. Denominator is always positive
3. Zero is represented as `0/1`

**Rationale:** 
- Ensures unique representation for equality comparison
- Simplifies implementation of comparison operators
- Matches mathematical conventions
- Prevents gradual growth of numerators and denominators

### 4.3 Zero Denominator

Construction or assignment with a zero denominator throws `std::domain_error`:

```cpp
std::rational r{1, 0};  // throws std::domain_error
```

**Rationale:**
- Division by zero is undefined in mathematics
- Throwing an exception is consistent with other standard library components
- Allows for constexpr construction (preconditions checked at compile time)
- Clear error reporting rather than undefined behavior

### 4.4 Reciprocal Operator

A unary `operator~` provides reciprocal (multiplicative inverse):

```cpp
std::rational r{3, 4};
auto reciprocal = ~r;  // 4/3
```

**Rationale:**
- Natural notation for reciprocal (complement operator)
- Convenient for division: `a / b` can be written as `a * ~b`
- Symmetric with unary `-` for additive inverse
- Similar to Julia's rational implementation

### 4.5 Floating-Point Conversion

Conversion from floating-point uses exact representation via `std::frexp`:

```cpp
std::rational r{0.5};  // Exactly 1/2, not an approximation
```

**Rationale:**
- Preserves exact binary representation of floating-point values
- No loss of information for exactly representable values
- Predictable behavior
- Users can implement approximate conversions separately if needed

### 4.6 Constexpr Support

All operations are `constexpr` where possible:

```cpp
constexpr std::rational r1{1, 2};
constexpr std::rational r2{1, 3};
constexpr auto sum = r1 + r2;  // Compile-time computation
static_assert(sum == std::rational{5, 6});
```

**Rationale:**
- Enables compile-time computation and validation
- Supports template metaprogramming use cases
- Consistent with modern C++ design principles
- Zero runtime overhead for constant expressions

### 4.7 Comparison Operators

Three-way comparison using `operator<=>` (C++20):

```cpp
std::rational r1{1, 2};
std::rational r2{2, 3};
auto cmp = r1 <=> r2;  // std::strong_ordering::less
```

**Rationale:**
- Leverages C++20 spaceship operator for concise implementation
- Generates all six comparison operators automatically
- Efficient implementation avoiding cross-multiplication overflow
- Provides strong ordering for use in ordered containers

### 4.8 User-Defined Literals

Optional literals in an inline namespace:

```cpp
using namespace std::rational_literals;
auto r = 42_r;  // std::rational<int>{42}
```

**Rationale:**
- Convenience for common use cases
- Opt-in to avoid namespace pollution
- Consistent with existing literals (`s`, `ms`, `ns`, etc.)
- Multiple variants for different integer types (`r`, `lr`, `llr`, `ur`, `ulr`, `ullr`)

### 4.9 Stream I/O

Standard stream operators with `numerator/denominator` format:

```cpp
std::rational r{3, 4};
std::cout << r;  // Outputs: 3/4

std::rational r2;
std::cin >> r2;  // Reads: 3/4 or just 3 (denominator defaults to 1)
```

**Rationale:**
- Natural mathematical notation
- Consistent with user expectations
- Supports round-trip serialization
- Respects stream formatting flags

### 4.10 Mathematical Functions

Limited mathematical function support:

```cpp
std::rational r{-3, 4};
auto absolute = std::abs(r);  // 3/4

std::rational base{2, 3};
auto squared = std::pow(base, 2);  // 4/9
```

**Rationale:**
- Only functions that preserve rational numbers are included
- Transcendental functions (sin, cos, log, etc.) would require approximation
- Users can convert to floating-point for such operations
- Extensible design allows future additions

## 5. Technical Specification

### 5.1 Header `<rational>` Synopsis

```cpp
namespace std {
  // Concepts
  template <typename T>
  concept integral = is_integral_v<T>;
  
  // Class template rational
  template <integral T>
  class rational;
  
  // Arithmetic operators
  template <integral T>
  constexpr rational<T> operator+(const rational<T>& lhs, const rational<T>& rhs);
  
  template <integral T>
  constexpr rational<T> operator-(const rational<T>& lhs, const rational<T>& rhs);
  
  template <integral T>
  constexpr rational<T> operator*(const rational<T>& lhs, const rational<T>& rhs);
  
  template <integral T>
  constexpr rational<T> operator/(const rational<T>& lhs, const rational<T>& rhs);
  
  // Mixed-type arithmetic
  template <integral T>
  constexpr rational<T> operator+(const rational<T>& lhs, const T& rhs);
  
  template <integral T>
  constexpr rational<T> operator+(const T& lhs, const rational<T>& rhs);
  
  template <integral T>
  constexpr rational<T> operator-(const rational<T>& lhs, const T& rhs);
  
  template <integral T>
  constexpr rational<T> operator-(const T& lhs, const rational<T>& rhs);
  
  template <integral T>
  constexpr rational<T> operator*(const rational<T>& lhs, const T& rhs);
  
  template <integral T>
  constexpr rational<T> operator*(const T& lhs, const rational<T>& rhs);
  
  template <integral T>
  constexpr rational<T> operator/(const rational<T>& lhs, const T& rhs);
  
  template <integral T>
  constexpr rational<T> operator/(const T& lhs, const rational<T>& rhs);
  
  // Stream operators
  template <typename CharT, typename Traits, integral T>
  basic_ostream<CharT, Traits>& operator<<(basic_ostream<CharT, Traits>& os, 
                                            const rational<T>& r);
  
  template <typename CharT, typename Traits, integral T>
  basic_istream<CharT, Traits>& operator>>(basic_istream<CharT, Traits>& is, 
                                            rational<T>& r);
  
  // Conversion functions
  template <arithmetic To, integral From>
  constexpr To rational_cast(const rational<From>& r);
  
  template <integral To, arithmetic From>
  constexpr rational<To> rational_cast(const From& value);
  
  // Accessor functions
  template <integral T>
  constexpr T numerator(const rational<T>& r);
  
  template <integral T>
  constexpr T denominator(const rational<T>& r);
  
  // Mathematical functions
  template <integral T>
  constexpr rational<T> abs(const rational<T>& r);
  
  template <integral T>
  constexpr rational<T> pow(const rational<T>& base, const T& exp);
  
  // User-defined literals
  inline namespace literals {
  inline namespace rational_literals {
    constexpr rational<int> operator""_r(unsigned long long val);
    constexpr rational<long> operator""_lr(unsigned long long val);
    constexpr rational<long long> operator""_llr(unsigned long long val);
    constexpr rational<unsigned int> operator""_ur(unsigned long long val);
    constexpr rational<unsigned long> operator""_ulr(unsigned long long val);
    constexpr rational<unsigned long long> operator""_ullr(unsigned long long val);
  }
  }
}
```

### 5.2 Class Template `rational`

```cpp
template <std::integral T>
class rational {
public:
  using value_type = T;
  
  // Constructors
  constexpr rational() noexcept;
  constexpr rational(const T& numerator, const T& denominator = T{1});
  
  template <floating_point U>
  constexpr explicit rational(const U& value);
  
  constexpr rational(const rational&) = default;
  constexpr rational(rational&&) = default;
  
  // Destructor
  ~rational() = default;
  
  // Assignment operators
  constexpr rational& operator=(const rational&) = default;
  constexpr rational& operator=(rational&&) = default;
  constexpr rational& operator=(const T& value);
  
  // Compound assignment operators
  constexpr rational& operator+=(const rational& rhs);
  constexpr rational& operator-=(const rational& rhs);
  constexpr rational& operator*=(const rational& rhs);
  constexpr rational& operator/=(const rational& rhs);
  
  constexpr rational& operator+=(const T& rhs);
  constexpr rational& operator-=(const T& rhs);
  constexpr rational& operator*=(const T& rhs);
  constexpr rational& operator/=(const T& rhs);
  
  // Increment and decrement
  constexpr rational& operator++();
  constexpr rational& operator--();
  constexpr rational operator++(int);
  constexpr rational operator--(int);
  
  // Unary operators
  constexpr rational operator+() const;
  constexpr rational operator-() const;
  constexpr rational operator~() const;  // Reciprocal
  
  // Comparison operators
  constexpr bool operator==(const rational& rhs) const = default;
  constexpr bool operator==(const T& rhs) const;
  constexpr strong_ordering operator<=>(const rational& rhs) const;
  constexpr strong_ordering operator<=>(const T& rhs) const;
  
  // Accessors
  [[nodiscard]] constexpr T numerator() const noexcept;
  [[nodiscard]] constexpr T denominator() const noexcept;
  
  // Mutators
  constexpr void numerator(const T& value);
  constexpr void denominator(const T& value);
  constexpr void assign(const T& numerator, const T& denominator);
  
  template <floating_point U>
  constexpr void assign(const U& value);
  
  // Conversion
  template <arithmetic U>
  [[nodiscard]] constexpr U convert() const;
  
private:
  T numerator_;
  T denominator_;
  
  constexpr void canonicalize();  // Reduce to lowest terms
};
```

### 5.3 Semantics

#### Constructor `rational(const T& numerator, const T& denominator)`

**Effects:** Constructs a rational number with the specified numerator and denominator, reduced to canonical form.

**Throws:** `std::domain_error` if `denominator == 0`.

**Postconditions:** 
- `this->numerator()` and `this->denominator()` are coprime
- `this->denominator() > 0`

**Complexity:** O(log(min(numerator, denominator)))

#### Constructor `rational(const U& value)` (floating-point)

**Effects:** Constructs a rational number representing the exact value of the floating-point parameter.

**Throws:** `std::domain_error` if `value` is infinite or NaN.

**Notes:** The resulting rational number exactly represents the binary floating-point value, which may not match the decimal representation.

**Example:**
```cpp
rational r{0.1};  // May not equal 1/10 due to binary representation
```

#### Member function `canonicalize()`

**Effects:** Reduces the rational number to canonical form:
1. Divides numerator and denominator by their GCD
2. If denominator is negative, negates both numerator and denominator

**Postconditions:**
- `gcd(numerator(), denominator()) == 1`
- `denominator() > 0`

#### Arithmetic operators

All arithmetic operators maintain the canonical form invariant.

**Throws:** `operator/=` and `operator/` throw `std::domain_error` when dividing by zero.

### 5.4 Free Functions

#### Function template `rational_cast`

```cpp
template <arithmetic To, integral From>
constexpr To rational_cast(const rational<From>& r);
```

**Returns:** `static_cast<To>(r.numerator()) / static_cast<To>(r.denominator())`

**Example:**
```cpp
rational r{1, 3};
double d = rational_cast<double>(r);  // 0.333...
```

```cpp
template <integral To, arithmetic From>
constexpr rational<To> rational_cast(const From& value);
```

**Returns:** `rational<To>(value)`

**Example:**
```cpp
auto r = rational_cast<rational<int>>(3.14);
```

#### Function template `abs`

```cpp
template <integral T>
constexpr rational<T> abs(const rational<T>& r);
```

**Returns:** The absolute value of `r`.

**Example:**
```cpp
rational r{-3, 4};
auto a = abs(r);  // 3/4
```

#### Function template `pow`

```cpp
template <integral T>
constexpr rational<T> pow(const rational<T>& base, const T& exp);
```

**Returns:** `rational<T>(std::pow(base.numerator(), exp), std::pow(base.denominator(), exp))`

**Requires:** `exp >= 0`

**Example:**
```cpp
rational r{2, 3};
auto squared = pow(r, 2);  // 4/9
```

## 6. Implementation Experience

### 6.1 Reference Implementation

A complete reference implementation is available at:
https://github.com/acdemiralp/rational

This implementation has been tested with:
- GCC 11+ with C++20 support
- Clang 12+ with C++20 support  
- MSVC 19.28+ with C++20 support

### 6.2 Performance Characteristics

Benchmark results show that `std::rational` operations are typically:
- 2-5x slower than floating-point for arithmetic operations
- Comparable to or faster than `double` for exact comparisons
- Significantly faster than decimal libraries for exact arithmetic

Memory usage:
- `sizeof(std::rational<int>)` = 8 bytes (two 4-byte integers)
- `sizeof(std::rational<int64_t>)` = 16 bytes (two 8-byte integers)

### 6.3 Existing Usage

The reference implementation has been used in:
- Computer graphics applications for exact geometric calculations
- Audio processing software for precise timing computations
- Educational software demonstrating exact arithmetic

### 6.4 Comparison with Boost.Rational

The proposed `std::rational` is heavily inspired by `boost::rational` with key differences:

| Feature | boost::rational | std::rational (proposed) |
|---------|----------------|-------------------------|
| C++ Version | C++03 | C++20 |
| Concepts | No | Yes |
| Spaceship Operator | No | Yes |
| Constexpr | Limited | Comprehensive |
| Reciprocal Operator | No | Yes (`operator~`) |
| Floating-Point Conversion | Approximate | Exact |
| User-Defined Literals | No | Yes |

## 7. Proposed Wording

The following wording is relative to N4971 (C++23 working draft).

### 7.1 Add to [numerics.general]

In **26.2 [numerics.general]**, add to the header summary:

```
26.X    Rational numbers           <rational>
```

### 7.2 Add new section [rational]

**26.X Rational numbers [rational]**

**26.X.1 Header `<rational>` synopsis [rational.syn]**

```cpp
namespace std {
  // 26.X.2, class template rational
  template <integral T>
  class rational;
  
  // 26.X.3, rational arithmetic
  template <integral T>
  constexpr rational<T> operator+(const rational<T>& lhs, const rational<T>& rhs);
  template <integral T>
  constexpr rational<T> operator-(const rational<T>& lhs, const rational<T>& rhs);
  template <integral T>
  constexpr rational<T> operator*(const rational<T>& lhs, const rational<T>& rhs);
  template <integral T>
  constexpr rational<T> operator/(const rational<T>& lhs, const rational<T>& rhs);
  
  // 26.X.4, rational mixed-type arithmetic
  template <integral T>
  constexpr rational<T> operator+(const rational<T>& lhs, const T& rhs);
  template <integral T>
  constexpr rational<T> operator+(const T& lhs, const rational<T>& rhs);
  // ... (similar for -, *, /)
  
  // 26.X.5, rational I/O
  template <typename CharT, typename Traits, integral T>
  basic_ostream<CharT, Traits>& operator<<(basic_ostream<CharT, Traits>& os, 
                                            const rational<T>& r);
  template <typename CharT, typename Traits, integral T>
  basic_istream<CharT, Traits>& operator>>(basic_istream<CharT, Traits>& is, 
                                            rational<T>& r);
  
  // 26.X.6, rational conversion
  template <arithmetic To, integral From>
  constexpr To rational_cast(const rational<From>& r);
  template <integral To, arithmetic From>
  constexpr rational<To> rational_cast(const From& value);
  
  // 26.X.7, rational accessors
  template <integral T>
  constexpr T numerator(const rational<T>& r);
  template <integral T>
  constexpr T denominator(const rational<T>& r);
  
  // 26.X.8, rational mathematical functions
  template <integral T>
  constexpr rational<T> abs(const rational<T>& r);
  template <integral T>
  constexpr rational<T> pow(const rational<T>& base, const T& exp);
  
  // 26.X.9, rational literals
  inline namespace literals {
  inline namespace rational_literals {
    constexpr rational<int> operator""_r(unsigned long long val);
    constexpr rational<long> operator""_lr(unsigned long long val);
    constexpr rational<long long> operator""_llr(unsigned long long val);
    constexpr rational<unsigned int> operator""_ur(unsigned long long val);
    constexpr rational<unsigned long> operator""_ulr(unsigned long long val);
    constexpr rational<unsigned long long> operator""_ullr(unsigned long long val);
  }
  }
}
```

**26.X.2 Class template rational [rational.class]**

```cpp
template <integral T>
class rational {
public:
  using value_type = T;
  
  // 26.X.2.1, constructors
  constexpr rational() noexcept;
  constexpr rational(const T& numerator, const T& denominator = T{1});
  template <floating_point U>
  constexpr explicit rational(const U& value);
  constexpr rational(const rational&) = default;
  constexpr rational(rational&&) = default;
  
  // 26.X.2.2, destructor
  ~rational() = default;
  
  // 26.X.2.3, assignment
  constexpr rational& operator=(const rational&) = default;
  constexpr rational& operator=(rational&&) = default;
  constexpr rational& operator=(const T& value);
  
  // 26.X.2.4, compound assignment
  constexpr rational& operator+=(const rational& rhs);
  constexpr rational& operator-=(const rational& rhs);
  constexpr rational& operator*=(const rational& rhs);
  constexpr rational& operator/=(const rational& rhs);
  constexpr rational& operator+=(const T& rhs);
  constexpr rational& operator-=(const T& rhs);
  constexpr rational& operator*=(const T& rhs);
  constexpr rational& operator/=(const T& rhs);
  
  // 26.X.2.5, increment and decrement
  constexpr rational& operator++();
  constexpr rational& operator--();
  constexpr rational operator++(int);
  constexpr rational operator--(int);
  
  // 26.X.2.6, unary operators
  constexpr rational operator+() const;
  constexpr rational operator-() const;
  constexpr rational operator~() const;
  
  // 26.X.2.7, comparison
  constexpr bool operator==(const rational& rhs) const = default;
  constexpr bool operator==(const T& rhs) const;
  constexpr strong_ordering operator<=>(const rational& rhs) const;
  constexpr strong_ordering operator<=>(const T& rhs) const;
  
  // 26.X.2.8, accessors and mutators
  [[nodiscard]] constexpr T numerator() const noexcept;
  [[nodiscard]] constexpr T denominator() const noexcept;
  constexpr void numerator(const T& value);
  constexpr void denominator(const T& value);
  constexpr void assign(const T& numerator, const T& denominator);
  template <floating_point U>
  constexpr void assign(const U& value);
  
  // 26.X.2.9, conversion
  template <arithmetic U>
  [[nodiscard]] constexpr U convert() const;
  
private:
  T numerator_;    // exposition only
  T denominator_;  // exposition only
  
  constexpr void canonicalize();  // exposition only
};
```

1. The class template `rational<T>` represents a rational number as a ratio of two integers of type `T`.

2. A `rational` object is maintained in canonical form:
   - The numerator and denominator are coprime (their greatest common divisor is 1)
   - The denominator is always positive
   - Zero is represented as `0/1`

3. All member functions and operators maintain the canonical form invariant.

**26.X.2.1 Constructors [rational.cons]**

```cpp
constexpr rational() noexcept;
```

1. *Effects:* Constructs a rational number representing zero.
2. *Postconditions:* `numerator() == 0` and `denominator() == 1`.

```cpp
constexpr rational(const T& numerator, const T& denominator = T{1});
```

1. *Effects:* Constructs a rational number from the given numerator and denominator, reduced to canonical form.
2. *Throws:* `domain_error` if `denominator == 0`.
3. *Postconditions:* `gcd(this->numerator(), this->denominator()) == 1` and `this->denominator() > 0`.
4. *Complexity:* O(log(min(|numerator|, |denominator|)))

```cpp
template <floating_point U>
constexpr explicit rational(const U& value);
```

1. *Effects:* Constructs a rational number representing the exact binary value of the floating-point parameter.
2. *Throws:* `domain_error` if `!isfinite(value)`.
3. *Remarks:* The resulting rational may not match the decimal representation of the floating-point value due to binary representation limitations.

**26.X.2.4 Compound assignment [rational.compound]**

```cpp
constexpr rational& operator/=(const rational& rhs);
constexpr rational& operator/=(const T& rhs);
```

1. *Effects:* Divides `*this` by `rhs` and stores the result in `*this`.
2. *Throws:* `domain_error` if `rhs` represents zero.
3. *Returns:* `*this`.

### 7.3 Feature test macro

Add to **15.11 [cpp.predefined]**:

```cpp
__cpp_lib_rational  202601L  // also in <rational>
```

## 8. Acknowledgements

This proposal draws inspiration from several sources:

- **Boost.Rational** by Paul Moore, which has served the C++ community for over 20 years
- **Julia's Rational** type, demonstrating elegant integration of rationals into a numeric tower
- **Python's fractions.Fraction**, showing the value of rational support in a general-purpose language
- The **reference implementation** by Ali Can Demiralp, which validated the design

Special thanks to the ISO C++ standardization committee for their ongoing work to improve the language.

## 9. References

1. Boost.Rational: https://www.boost.org/doc/libs/release/libs/rational/
2. Julia Rational: https://docs.julialang.org/en/v1/base/numbers/#Base.Rational
3. Python fractions: https://docs.python.org/3/library/fractions.html
4. Reference Implementation: https://github.com/acdemiralp/rational
5. ISO/IEC 14882:2023 - Programming Languages — C++
6. P0192R1 - "Introducing `std::any`" (as a style reference for proposals)
7. Knuth, Donald E. "The Art of Computer Programming, Volume 2: Seminumerical Algorithms"
8. Graham, Ronald L., et al. "Concrete Mathematics: A Foundation for Computer Science"

---

## Revision History

### R0 (2026-01-28)
- Initial proposal
- Complete technical specification
- Reference implementation available
- Proposed wording for standardization
