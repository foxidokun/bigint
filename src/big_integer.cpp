#include "big_integer.hpp"

#include <stdint.h>

#include <algorithm>
#include <cassert>
#include <climits>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <vector>

// ----------------------------------------------------------------------------

static void AddBuffers(std::vector<uint32_t>& lhs,
                       const std::vector<uint32_t>& rhs);

[[nodiscard("You should check for sign change")]] static BigInt::Sign
SubBuffers(std::vector<uint32_t>& left_op,
           const std::vector<uint32_t>& right_op);

static std::strong_ordering CompareBuffers(const std::vector<uint32_t>& lhs,
                                           const std::vector<uint32_t>& rhs);

// ----------------------------------------------------------------------------

namespace {
template <typename T>
constexpr std::size_t BitSize() noexcept {
  return sizeof(T) * CHAR_BIT;
}

constexpr uint64_t kDecBase = 10;
};  // namespace

// ----------------------------------------------------------------------------

BigInt::BigInt(int64_t val) {
  if (val == 0) {
    return;
  }

  uint64_t val_abs;

  if (val > 0) {
    val_abs = static_cast<uint64_t>(val);
    sign_ = Sign::Positive;
  } else {
    val_abs = (val != INT64_MIN) ? -val : static_cast<uint64_t>(INT64_MAX) + 1;
    sign_ = Sign::Negative;
  }

  digits_.emplace_back(val_abs & UINT32_MAX);
  if (val_abs > UINT32_MAX) {
    val_abs >>= BitSize<uint32_t>();
    digits_.emplace_back(val_abs);
  }
}

BigInt::BigInt(std::string_view decimal_input) {
  const auto* it = decimal_input.begin();
  if (*it == '-') {
    sign_ = Sign::Negative;
    ++it;
  }

  BigInt tmp(0);

  for (const auto* end = decimal_input.end(); it < end; ++it) {
    tmp *= kDecBase;
    tmp += (*it - '0');
  }

  digits_ = std::move(tmp.digits_);

  // sign correcting
  if (sign_ != Sign::Negative && !digits_.empty()) {
    sign_ = Sign::Positive;
  } else if (sign_ == Sign::Negative && digits_.empty()) {
    sign_ = Sign::Zero;
  }
}

BigInt& BigInt::operator+=(const BigInt& other) {
  // Math optimizations
  if (other.sign_ == Sign::Zero) {
    return *this;
  }

  if (sign_ == Sign::Zero) {
    *this = other;
    return *this;
  }

  // Check signs
  if (sign_ == other.sign_) {
    AddBuffers(digits_, other.digits_);
  } else {
    auto res_sign = SubBuffers(digits_, other.digits_);
    sign_ = sign_ * res_sign;
  }

  return *this;
}

BigInt& BigInt::operator-=(const BigInt& other) {
  // Math optimizations
  if (other.sign_ == Sign::Zero) {
    return *this;
  }

  if (sign_ == Sign::Zero) {
    *this = -other;
    return *this;
  }

  // Check signs
  if (sign_ == other.sign_) {
    auto res_sign = SubBuffers(digits_, other.digits_);
    sign_ = sign_ * res_sign;
  } else {
    AddBuffers(digits_, other.digits_);
  }

  return *this;
}

// NOLINTNEXTLINE // temporary
BigInt& BigInt::operator*=(const BigInt& other) {
  if (sign_ == Sign::Zero) {
    return *this;
  }

  if (other.sign_ == Sign::Zero) {
    *this = BigInt(0);
    return *this;
  }

  sign_ = sign_ * other.sign_;

  std::vector<uint32_t> new_digits(digits_.size() + other.digits_.size(), 0);

  for (uint64_t i = 0; i < digits_.size(); ++i) {
    for (uint64_t j = 0; j < other.digits_.size(); ++j) {
      uint64_t carry = static_cast<uint64_t>(digits_[i]) * other.digits_[j];

      for (size_t k = i + j; carry > 0; ++k) {
        assert(k < new_digits.size());

        carry += new_digits[k];
        new_digits[k] = carry & UINT32_MAX;
        carry >>= BitSize<uint32_t>();
      }
    }
  }

  digits_ = std::move(new_digits);

  while (digits_[digits_.size() - 1] == 0) {
    digits_.pop_back();
  }

  return *this;
}

BigInt BigInt::GetDivDigit(const BigInt& other) const {
  uint64_t head_lhs = GetHead();
  uint64_t head_rhs = other.GetHead((int64_t)digits_.size() - 2);

  if (head_rhs != UINT64_MAX) {
    return BigInt(std::min(head_lhs / (head_rhs + 1), (uint64_t)INT64_MAX));
  } /* else */  // due to clang-tidy

  assert(head_rhs == head_lhs);
  return BigInt(1);
}

// TODO: refactor
// NOLINTNEXTLINE // temporary
BigInt& BigInt::operator/=(const BigInt& other) {
  if (other == BigInt(1)) {
    return *this;
  }

  if (sign_ == Sign::Zero) {
    return *this;
  }

  if (CompareBuffers(digits_, other.digits_) == std::strong_ordering::less) {
    *this = BigInt(0);
    return *this;
  }

  // Be positive
  auto res_sign = sign_ * other.sign_;
  sign_ = Sign::Positive;

  BigInt div_result = BigInt(0);

  std::strong_ordering cmp = std::strong_ordering::equal;

  while ((cmp = CompareBuffers(digits_, other.digits_)) ==
         std::strong_ordering::greater) {
    BigInt cur_sub = other;
    cur_sub.sign_ = Sign::Positive;

    uint64_t zero_cnt =
        std::max(digits_.size() - other.digits_.size(), 1UL) - 1;

    cur_sub.LeftShift(zero_cnt);
    BigInt div_digit = GetDivDigit(cur_sub);

    cur_sub *= div_digit;
    div_digit.LeftShift(zero_cnt);

    *this -= cur_sub;
    div_result += div_digit;
  }

  if (cmp == std::strong_ordering::equal) {
    div_result += 1;
  }

  *this = std::move(div_result);
  sign_ = res_sign;
  return *this;
}

uint64_t BigInt::GetHead(int64_t start_index) const {
  uint64_t ans = 0;

  int64_t num_of_digits = static_cast<int64_t>(digits_.size());

  if (!digits_.empty() && start_index <= num_of_digits - 1) {
    ans += digits_[digits_.size() - 1];
  }

  if (digits_.size() > 1 && start_index <= num_of_digits - 2) {
    ans <<= BitSize<uint32_t>();
    ans |= digits_[digits_.size() - 2];
  }

  return ans;
}

BigInt& BigInt::operator%=(const BigInt& other) {
  *this = *this - (*this / other) * other;
  return *this;
}

BigInt& BigInt::operator+=(int32_t other) {
  if (other == 0) {
    return *this;
  }

  if (sign_ == Sign::Zero) {
    *this = BigInt(other);
    return *this;
  }

  if (!IsSameSignAs(other)) {
    *this -= -other;
    return *this;
  }

  uint64_t carry = (other > 0) ? other : -other;

  for (auto& digit : digits_) {
    carry += digit;
    digit = carry & UINT32_MAX;
    carry >>= BitSize<uint32_t>();
  }

  if (carry > 0) {
    digits_.emplace_back(carry);
  }

  return *this;
}

// NOLINTNEXTLINE // temporary
BigInt& BigInt::operator-=(int32_t other) {
  if (other == 0) {
    return *this;
  }

  if (sign_ == Sign::Zero) {
    *this = BigInt(-other);
    return *this;
  }

  uint64_t carry = (other > 0) ? other : -other;

  if (!IsSameSignAs(other)) {
    *this += -other;
    return *this;
  }

  if (digits_.size() == 1) {
    if (digits_[0] > carry) {
      digits_[0] -= carry;
    } else if (digits_[0] < carry) {
      digits_[0] = carry - digits_[0];
      sign_ = OppositeSign(sign_);
    } else {
      digits_.clear();
      sign_ = Sign::Zero;
    }

    return *this;
  }

  for (auto& digit : digits_) {
    if (digit >= carry) {
      digit = digit - carry;
      carry = 0;
    } else {
      digit = UINT32_MAX - carry + digit;
      carry = 1;
    }
  }

  // GC if needed
  if (digits_[digits_.size() - 1] == 0) {
    digits_.pop_back();
  }

  assert(carry == 0);
  assert(digits_[digits_.size() - 1] != 0);

  return *this;
}

BigInt& BigInt::operator*=(int32_t other) {
  if (sign_ == Sign::Zero) {
    return *this;
  }

  if (other == 0) {
    *this = BigInt(0);
    return *this;
  }

  if (other < 0) {
    sign_ = OppositeSign(sign_);
    other = -other;
  }

  uint64_t carry = 0;

  for (auto& digit : digits_) {
    carry += static_cast<uint64_t>(digit) * other;
    digit = carry & UINT32_MAX;
    carry >>= BitSize<uint32_t>();
  }

  if (carry > 0) {
    digits_.emplace_back(carry);
  }

  return *this;
}

BigInt& BigInt::operator++() {
  *this += 1;
  return *this;
}

BigInt& BigInt::operator--() {
  *this -= 1;
  return *this;
}

BigInt BigInt::operator++(int) {
  BigInt copy = *this;
  *this += 1;
  return copy;
}

BigInt BigInt::operator--(int) {
  BigInt copy = *this;
  *this -= 1;
  return copy;
}

BigInt BigInt::operator-() const {
  BigInt copy = *this;
  copy.sign_ = OppositeSign(sign_);
  return copy;
}

std::strong_ordering BigInt::operator<=>(const BigInt& other) const {
  if (sign_ == other.sign_) {
    if (sign_ == Sign::Zero) {
      return std::strong_ordering::equal;
    }

    auto buf_cmp = CompareBuffers(digits_, other.digits_);

    // Thanks to clang-tidy, i can't write if, so eat this
    return sign_ == Sign::Positive ? buf_cmp : nullptr <=> buf_cmp;
  }

  switch (sign_) {
    case Sign::Positive:
      return std::strong_ordering::greater;
    case Sign::Negative:
      return std::strong_ordering::less;
    case Sign::Zero:
      return (other.sign_ == Sign::Negative) ? std::strong_ordering::greater
                                             : std::strong_ordering::less;
  }

  assert(false);
}

bool BigInt::operator==(const BigInt& other) const {
  if (sign_ != other.sign_ || digits_.size() != other.digits_.size()) {
    return false;
  }

  return std::equal(digits_.cbegin(), digits_.cend(), other.digits_.cbegin());
}

BigInt::Sign BigInt::OppositeSign(Sign sign) {
  switch (sign) {
    case Sign::Negative:
      return Sign::Positive;
    case Sign::Zero:
      return Sign::Zero;
    case Sign::Positive:
      return Sign::Negative;
  }

  assert(false);  // unreachable
}

bool BigInt::IsSameSignAs(int32_t val) {
  return (val < 0 && sign_ == Sign::Negative) ||
         (val > 0 && sign_ == Sign::Positive) ||
         (val == 0 && sign_ == Sign::Zero);
}

void BigInt::LeftShift(uint32_t digit_num) {
  if (digit_num == 0) {
    return;
  }

  // Allocate mem
  digits_.resize(digits_.size() + digit_num);

  // Shift digits
  for (size_t i = digits_.size() - 1; i >= digit_num; --i) {
    digits_[i] = digits_[i - digit_num];
  }

  // Zerofy smallest digits
  for (size_t i = 0; i < digit_num; ++i) {
    digits_[i] = 0;
  }
}

// NOLINTNEXTLINE // temporary
static void AddBuffers(std::vector<uint32_t>& lhs,
                       const std::vector<uint32_t>& rhs) {
  uint64_t carry = 0;

  // Reserve space so we won't invalidate iterators
  // NOTE: We should be carefull if size(lhs + rhs) > max(size(lhs), size(rhs))
  // and it would cause reallocation
  //   you can track it by «Dangerous case goes here» comments
  lhs.reserve(rhs.size());

  // Precompute iterators
  auto lhs_it = lhs.begin();
  auto lhs_end = lhs.end();

  auto rhs_it = rhs.cbegin();
  auto rhs_end = rhs.cend();

  // Process common
  for (; lhs_it < lhs_end && rhs_it < rhs_end; ++lhs_it, ++rhs_it) {
    carry =
        static_cast<uint64_t>(*lhs_it) + static_cast<uint64_t>(*rhs_it) + carry;
    *lhs_it = carry & UINT32_MAX;
    carry >>= BitSize<uint32_t>();
  }

  // Case 1: size(lhs) >= size(rhs)
  // NOTE: Dangerous case goes here
  if (rhs_it == rhs_end) {
    // Add last carry
    // NOTE: Dangerous case skip this loop
    for (; carry > 0 && lhs_it != lhs_end; ++lhs_it) {
      carry += *lhs_it;
      *lhs_it = carry & UINT32_MAX;
      carry >>= BitSize<uint32_t>();
    }

    // NOTE: Dangerous case goes here
    if (carry != 0) {
      assert(lhs_it == lhs_end);
      lhs.emplace_back(carry);
    }

    return;
  }

  for (; rhs_it != rhs_end; ++rhs_it) {
    carry += *rhs_it;
    lhs.emplace_back(carry & UINT32_MAX);
    carry >>= BitSize<uint32_t>();
  }
}

[[nodiscard("You should check for sign = zero")]] static BigInt::Sign
// NOLINTNEXTLINE // temporary
SubBuffersTo(const std::vector<uint32_t>& lhs, const std::vector<uint32_t>& rhs,
             std::vector<uint32_t>& out) {
  out.reserve(lhs.size());

  uint64_t carry = 0;
  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();
  auto out_it = out.begin();
  auto lhs_end = lhs.end();  // оно было бы константным, если бы не clang tidy
  auto rhs_end = rhs.end();  // оно было бы константным, если бы не clang tidy

  // Substraction phase
  for (; lhs_it < lhs_end && rhs_it < rhs_end; ++lhs_it, ++rhs_it, ++out_it) {
    carry += *rhs_it;
    uint64_t digit = *lhs_it;

    if (digit >= carry) {
      *out_it = digit - carry;
      carry = 0;
    } else {
      *out_it = UINT32_MAX - carry + digit + 1;
      carry = 1;
    }
  }

  for (; lhs_it < lhs_end; ++lhs_it, ++out_it) {
    uint64_t digit = *lhs_it;

    if (digit >= carry) {
      *out_it = digit - carry;
      carry = 0;
    } else {
      *out_it = UINT32_MAX - carry + digit + 1;
      carry = 1;
    }
  }

  assert(carry == 0);

  // GC Stage
  while (!out.empty() && out[out.size() - 1] == 0) {
    out.pop_back();
  }

  return out.empty() ? BigInt::Sign::Zero : BigInt::Sign::Positive;
}

BigInt::Sign operator*(const BigInt::Sign& lhs, const BigInt::Sign& rhs) {
  if (lhs == BigInt::Sign::Zero || rhs == BigInt::Sign::Zero) {
    return BigInt::Sign::Zero;
  }

  return lhs == rhs ? BigInt::Sign::Positive : BigInt::Sign::Negative;
}

// breaking naming due to clang-tidy: readability-suspicious-call-argument
static BigInt::Sign SubBuffers(std::vector<uint32_t>& left_op,
                               const std::vector<uint32_t>& right_op) {
  // lhs >= rhs
  if (CompareBuffers(left_op, right_op) != std::strong_ordering::less) {
    auto sign = SubBuffersTo(left_op, right_op, left_op);
    return sign * BigInt::Sign::Positive;
  }
  /* else */  // due to cringe clang-tidy: readability-else-after-return

  auto sign = SubBuffersTo(right_op, left_op, left_op);
  return sign * BigInt::Sign::Negative;
}

static std::strong_ordering CompareBuffers(const std::vector<uint32_t>& lhs,
                                           const std::vector<uint32_t>& rhs) {
  if (lhs.size() > rhs.size()) {
    return std::strong_ordering::greater;
  }

  if (lhs.size() < rhs.size()) {
    return std::strong_ordering::less;
  }

  return std::lexicographical_compare_three_way(lhs.crbegin(), lhs.crend(),
                                                rhs.crbegin(), rhs.crend());
}

std::ostream& operator<<(std::ostream& stream, BigInt val) {
  BigInt base(kDecBase);
  std::string buf;

  while (val) {
    buf += (val % base).digits_[0];
    val /= base;
  }

  std::copy(buf.crbegin(), buf.crend(), std::ostream_iterator<int>(stream));

  return stream;
}

std::istream& operator>>(std::istream& stream, BigInt& val) {
  std::string buf;
  stream >> buf;

  val = BigInt(buf);

  return stream;
}
