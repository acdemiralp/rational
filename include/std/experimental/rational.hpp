#pragma once

#include <cmath>
#include <compare>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <type_traits>

namespace std::experimental
{
// Concepts corresponding to <type_traits> categories (may be extended to cover all primary/composite categories, properties, relations).
template <typename type>
concept arithmetic     = std::is_arithmetic_v    <type>;
template <typename type>
concept floating_point = std::is_floating_point_v<type>;
template <typename type>
concept integral       = std::is_integral_v      <type>;

// Limitations:
// - The denominator can not be zero (throws std::domain_error).
// Furthermore the rational is kept in canonical form:
// - The numerator and denominator are co-prime integers (have no common factors).
// - Denominator is greater than zero.
template <integral type>
class rational
{
public:
  // Constructors and destructor.
  constexpr rational         (const type& numerator = type(0), const type& denominator = type(1))
  : numerator_(numerator), denominator_(denominator)
  {
    if (denominator == type(0))
      throw std::domain_error("Denominator can not be zero.");

    canonize();
  }
  template <floating_point that_type>
  constexpr rational         (const that_type& that)
  {
    assign(that);
  }
  constexpr rational         (const rational&  that) = default;
  constexpr rational         (      rational&& temp) = default;
  constexpr virtual ~rational()                      = default;

  // Assignment operators.
  constexpr rational&            operator=  (const rational&  that) = default;
  constexpr rational&            operator=  (      rational&& temp) = default;
  constexpr rational&            operator=  (const type&      that)
  {
    numerator_   = that   ;
    denominator_ = type(1);
    return *this;
  }

  // Comparison operators.
  constexpr bool                 operator== (const rational&  that) const = default;
  constexpr bool                 operator== (const type&      that) const
  {
    return numerator_ == that && denominator_ == type(1);
  }
  constexpr std::strong_ordering operator<=>(const rational&  that) const
  {
    // a/b < c/d iff ad < bc.
    if (*this == that)
      return std::strong_ordering::equal;
    return numerator_ * that.denominator_ <=> denominator_ * that.numerator_;
  }
  constexpr std::strong_ordering operator<=>(const type&      that) const
  {
    // a/b < c/1 iff a < bc.
    if (*this == that)
      return std::strong_ordering::equal;
    return numerator_ <=> denominator_ * that;
  }

  // Unary arithmetic operators.
  constexpr rational             operator+  () const
  {
    return {*this};
  }
  constexpr rational             operator-  () const
  {
    return {-numerator_, denominator_};
  }
  constexpr rational             operator~  () const
  {
    return {denominator_, numerator_};
  }

  // Arithmetic assignment operators.
  constexpr rational&            operator+= (const rational&  that)
  {
    // a / b + c / d = (ad + bc) / bd
    numerator_   = numerator_   * that.denominator_ + denominator_ * that.numerator_;
    denominator_ = denominator_ * that.denominator_;
    canonize();
    return *this;
  }
  constexpr rational&            operator-= (const rational&  that)
  {
    // a / b - c / d = (ad - bc) / bd
    numerator_   = numerator_   * that.denominator_ - denominator_ * that.numerator_;
    denominator_ = denominator_ * that.denominator_;
    canonize();
    return *this;
  }
  constexpr rational&            operator*= (const rational&  that)
  {
    // a / b * c / d = ac / bd
    numerator_   *= that.numerator_  ;
    denominator_ *= that.denominator_;
    canonize();
    return *this;
  }
  constexpr rational&            operator/= (const rational&  that)
  {
    // a / b / c / d = ad / bc
    if (that.numerator_ == type(0))
      throw std::domain_error("Division by zero.");

    numerator_   *= that.denominator_;
    denominator_ *= that.numerator_  ;
    canonize();
    return *this;
  }
  constexpr rational&            operator+= (const type&      that)
  {
    // a / b + c / 1 = (a + bc) / b
    numerator_ += that * denominator_;
    return *this;
  }
  constexpr rational&            operator-= (const type&      that)
  {
    // a / b - c / 1 = (a - bc) / b
    numerator_ -= that * denominator_;
    return *this;
  }
  constexpr rational&            operator*= (const type&      that)
  {
    // a / b * c / 1 = ac / b
    numerator_ *= that;
    canonize();
    return *this;
  }
  constexpr rational&            operator/= (const type&      that)
  {
    // a / b / c / 1 = a / bc
    if (that == type(0))
      throw std::domain_error("Division by zero.");

    denominator_ *= that;
    canonize();
    return *this;
  }

  // Increment and decrement operators.
  constexpr rational&            operator++ ()
  {
    numerator_ += denominator_;
    return *this;
  }
  constexpr rational&            operator-- ()
  {
    numerator_ -= denominator_;
    return *this;
  }
  constexpr rational             operator++ (int)
  {
    rational result(*this);
    ++(*this);
    return result;
  }
  constexpr rational             operator-- (int)
  {
    rational result(*this);
    --(*this);
    return result;
  }

  // Accessor and mutators.
  [[nodiscard]]
  constexpr type numerator  () const
  {
    return numerator_;
  }
  constexpr void numerator  (const type& value)
  {
    numerator_   = value;
    canonize();
  }

  [[nodiscard]]
  constexpr type denominator() const
  {
    return denominator_;
  }
  constexpr void denominator(const type& value)
  {
    if (value == type(0))
      throw std::domain_error("Denominator can not be zero.");

    denominator_ = value;
    canonize();
  }

  constexpr void assign     (const type& numerator, const type& denominator)
  {
    if (denominator == type(0))
      throw std::domain_error("Denominator can not be zero.");

    numerator_   = numerator  ;
    denominator_ = denominator;
    canonize();
  }
  template <floating_point that_type>
  constexpr void assign     (const that_type& value)
  {
    // Reference: https://stackoverflow.com/questions/51142275/exact-value-of-a-floating-point-number-as-a-rational.
    static const auto mantissa         = std::numeric_limits<that_type>::digits;
    static const auto maximum_exponent = std::numeric_limits<that_type>::max_exponent;

    if (!std::isfinite(value))
      throw std::domain_error("Value can not be infinite.");

    auto exponent = 0;
    numerator_    = static_cast<type>(std::frexp(value, &exponent) * static_cast<that_type>(std::exp2(mantissa)));
    denominator_  = type(1);
    exponent     -= mantissa;

    if      (exponent > 0)
      numerator_ *= static_cast<type>(std::exp2(exponent));
    else if (exponent < 0)
    {
      exponent = -exponent;
      if (exponent >= maximum_exponent - 1)
      {
        numerator_   /= static_cast<type>(std::exp2(exponent - (maximum_exponent - 1)));
        denominator_ *= static_cast<type>(std::exp2(            maximum_exponent - 1 ));

        if (numerator_ == 0)
          throw std::domain_error("Value evaluates to zero due to being too small.");

        assign(numerator_, denominator_);
        return;
      }
      denominator_ *= static_cast<type>(std::exp2(exponent));
    }

    assign(numerator_, denominator_);
  }

  // Other functions.
  template <arithmetic result_type> [[nodiscard]]
  result_type    evaluate   () const
  {
    return static_cast<result_type>(numerator_) / static_cast<result_type>(denominator_);
  }
  
protected:
  // Canonical form implies that the numerator and denominator are co-prime integers (have no common factors) and the denominator is greater than zero.
  void           canonize   ()
  {
    const auto gcd = std::gcd(numerator_, denominator_);
    numerator_   /= gcd;
    denominator_ /= gcd;
    
    if (type(0) > denominator_)
    {
      numerator_   = -numerator_  ;
      denominator_ = -denominator_;
    }
  }

  type numerator_  ;
  type denominator_;
};

// Arithmetic operators.
template <integral type>
constexpr rational<type>               operator+      (const rational<type>& lhs, const type&           rhs)
{
  rational<type> result(lhs);
  return result += rhs;
}
template <integral type>
constexpr rational<type>               operator+      (const type&           lhs, const rational<type>& rhs)
{
  rational<type> result(rhs);
  return result += lhs;
}
template <integral type>
constexpr rational<type>               operator-      (const rational<type>& lhs, const type&           rhs)
{
  rational<type> result(lhs);
  return result -= rhs;
}
template <integral type>
constexpr rational<type>               operator-      (const type&           lhs, const rational<type>& rhs)
{
  rational<type> result(rhs);
  return -(result -= lhs);
}
template <integral type>
constexpr rational<type>               operator*      (const rational<type>& lhs, const type&           rhs)
{
  rational<type> result(lhs);
  return result *= rhs;
}
template <integral type>
constexpr rational<type>               operator*      (const type&           lhs, const rational<type>& rhs)
{
  rational<type> result(rhs);
  return result *= lhs;
}
template <integral type>
constexpr rational<type>               operator/      (const rational<type>& lhs, const type&           rhs)
{
  rational<type> result(lhs);
  return result /= rhs;
}
template <integral type>
constexpr rational<type>               operator/      (const type&           lhs, const rational<type>& rhs)
{
  rational<type> result(lhs);
  return result /= rhs;
}

// Stream operators.
template<typename char_type, typename traits, integral type>
std::basic_ostream<char_type, traits>& operator<<     (std::basic_ostream<char_type, traits>& stream, const rational<type>& value)
{
  stream << value.numerator() << '/' << value.denominator();
  return stream;
}
template<typename char_type, typename traits, integral type>
std::basic_istream<char_type, traits>& operator>>     (std::basic_istream<char_type, traits>& stream,       rational<type>& value)
{
  type numerator  (0);
  type denominator(1);

  auto flags = stream.flags(); // Save flag state.

  stream >> skipws;
  if (stream >> numerator)
    if (char slash; stream >> slash && slash == '/')
      stream >> denominator;

  stream.flags(flags); // Restore flag state.

  value.assign(numerator, denominator);
  return stream;
}

// Integer literals.
constexpr rational<int>                operator"" r   (const unsigned long long value)
{
  return {static_cast<int>          (value)};
}
constexpr rational<long>               operator"" lr  (const unsigned long long value)
{
  return {static_cast<long>         (value)};
}
constexpr rational<long long>          operator"" llr (const unsigned long long value)
{
  return {static_cast<long long>    (value)};
}
constexpr rational<unsigned int>       operator"" ur  (const unsigned long long value)
{
  return {static_cast<unsigned int> (value)};
}
constexpr rational<unsigned long>      operator"" ulr (const unsigned long long value)
{
  return {static_cast<unsigned long>(value)};
}
constexpr rational<unsigned long long> operator"" ullr(const unsigned long long value)
{
  return {value};
}

// Conversion functions to/from arithmetic type.
template <arithmetic arithmetic_type, integral   integral_type>
constexpr arithmetic_type         rational_cast(const rational<integral_type>& value)
{
  return value.template evaluate<arithmetic_type>();
}
template <integral   integral_type  , arithmetic arithmetic_type>
constexpr rational<integral_type> rational_cast(const arithmetic_type&         value)
{
  return rational<integral_type>(value);
}

// Uniform member access functions.
template <integral   type>
constexpr type                    numerator    (const rational<type>&          value)
{
  return value.numerator();
}
template <arithmetic type>
constexpr type                    numerator    (const type&                    value)
{
  return value;
}
template <integral   type>
constexpr type                    denominator  (const rational<type>&          value)
{
  return value.denominator();
}
template <arithmetic type>
constexpr type                    denominator  (const type&                    value)
{
  return type(1);
}

// Specializations for math functions.
template <integral type>
rational<type>                    abs          (const rational<type>&          value)
{
  return {std::abs(value.numerator()), value.denominator()};
}
template <integral type>
rational<type>                    pow          (const rational<type>&          value, const type& power)
{
  return {std::pow(value.numerator(), power), std::pow(value.denominator(), power)};
}

// TODO Potential: Specializations for more math functions.
// TODO Potential: Language modification operator\ (backslash may still be used as a line continuation in macros, and as a escape sequence in string literals)
// template <integral type>
// constexpr rational<type> operator\(type lhs, type rhs)
// {
//   return {lhs, rhs};
// }
}