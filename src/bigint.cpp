#include "bigint.hpp"
#include <algorithm>
#include <compare>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iterator>
#include <vector>

static void AddBuffers(std::vector<uint32_t>& lhs,
                       const std::vector<uint32_t>& rhs);

[[nodiscard("You should check for sign change")]] static BigInt::Sign
SubBuffers(std::vector<uint32_t>& lhs, const std::vector<uint32_t>& rhs);

static BigInt::Sign MulSign(BigInt::Sign lhs, BigInt::Sign rhs);

static std::strong_ordering CompareBuffers(const std::vector<uint32_t>& lhs,
                                           const std::vector<uint32_t>& rhs);

BigInt::BigInt(int64_t val) {
  if (val == 0) {
    return;
  }

  if (val > 0) {
    sign_ = Sign::Positive;
  } else {
    sign_ = Sign::Negative;
    val = -val;
  }

  digits_.emplace_back(val & UINT32_MAX);
  if (val > UINT32_MAX) {
    val >>= 32;
    digits_.emplace_back(val);
  }
}

BigInt::BigInt(std::string_view decimal_input) {
  auto it = decimal_input.begin();
  if (*it == '-') {
    sign_ = Sign::Negative;
    ++it;
  }

  BigInt tmp(0);

  for (auto end = decimal_input.end(); it < end; ++it) {
    tmp *= 10;
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
    sign_ = MulSign(res_sign, sign_);
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
    sign_ = MulSign(res_sign, sign_);
  } else {
    AddBuffers(digits_, other.digits_);
  }

  return *this;
}

BigInt& BigInt::operator*=(const BigInt& other) {
  if (sign_ == Sign::Zero) {
    return *this;
  }

  if (other.sign_ == Sign::Zero) {
    *this = BigInt(0);
    return *this;
  }

  sign_ = MulSign(sign_, other.sign_);

  std::vector<uint32_t> new_digits(digits_.size() + other.digits_.size(), 0);

  for (uint64_t i = 0; i < digits_.size(); ++i) {
    for (uint64_t j = 0; j < other.digits_.size(); ++j) {
      uint64_t carry = static_cast<uint64_t>(digits_[i]) * other.digits_[j];

      for (int k = i+j; carry > 0; ++k) {
        assert (k < new_digits.size());

        carry += new_digits[k];
        new_digits[k] = carry & UINT32_MAX;
        carry >>= 32;
      }
    }
  }

  digits_ = std::move(new_digits);

  while (digits_[digits_.size() - 1] == 0) {
    digits_.pop_back();
  }

  return *this;
}

BigInt& BigInt::operator/=(const BigInt& other) {
  /*Not implemented*/
  return *this;
}

BigInt& BigInt::operator%=(const BigInt& other) {
  /*Not implemented*/
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
    carry >>= 32;
  }

  if (carry > 0) {
    digits_.emplace_back(carry);
  }

  return *this;
}

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
    carry >>= 32;
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
  /*Not implemented*/
  return std::strong_ordering::equal;
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
}

bool BigInt::IsSameSignAs(int32_t val) {
  return (val < 0 && sign_ == Sign::Negative) ||
         (val > 0 && sign_ == Sign::Positive) ||
         (val == 0 && sign_ == Sign::Zero);
}

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
  const auto lhs_end = lhs.end();

  auto rhs_it = rhs.cbegin();
  const auto rhs_end = rhs.cend();

  // Process common
  for (; lhs_it < lhs_end && rhs_it < rhs_end; ++lhs_it, ++rhs_it) {
    carry =
        static_cast<uint64_t>(*lhs_it) + static_cast<uint64_t>(*rhs_it) + carry;
    *lhs_it = carry & UINT32_MAX;
    carry >>= 32;
  }

  // Case 1: size(lhs) >= size(rhs)
  // NOTE: Dangerous case goes here
  if (rhs_it == rhs_end) {
    // Add last carry
    // NOTE: Dangerous case skip this loop
    for (; carry > 0 && lhs_it != lhs_end; ++lhs_it) {
      carry += *lhs_it;
      *lhs_it = carry & UINT32_MAX;
      carry >>= 32;
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
    carry >>= 32;
  }
}

[[nodiscard("You should check for sign = zero")]] static BigInt::Sign
SubBuffersTo(const std::vector<uint32_t>& lhs, const std::vector<uint32_t>& rhs,
             std::vector<uint32_t>& out) {
  out.reserve(lhs.size());

  uint64_t carry = 0;
  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();
  auto out_it = out.begin();
  const auto lhs_end = lhs.end();
  const auto rhs_end = rhs.end();

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

  assert(carry == 0);

  // GC Stage
  while (!out.empty() && out[out.size() - 1] == 0) {
    out.pop_back();
  }

  if (out.empty()) {
    return BigInt::Sign::Zero;
  } else {
    return BigInt::Sign::Positive;
  }
}

static BigInt::Sign MulSign(BigInt::Sign lhs, BigInt::Sign rhs) {
  if (lhs == BigInt::Sign::Zero || rhs == BigInt::Sign::Zero) {
    return BigInt::Sign::Zero;
  }

  if (lhs == rhs) {
    return BigInt::Sign::Positive;
  } else {
    return BigInt::Sign::Negative;
  }
}

// return bool: needed sign change
static BigInt::Sign SubBuffers(std::vector<uint32_t>& lhs,
                               const std::vector<uint32_t>& rhs) {
  // lhs > rhs
  if (CompareBuffers(lhs, rhs) != std::strong_ordering::less) {
    auto sign = SubBuffersTo(lhs, rhs, lhs);
    return MulSign(sign, BigInt::Sign::Positive);
  } else {
    auto sign = SubBuffersTo(rhs, lhs, lhs);
    return MulSign(sign, BigInt::Sign::Negative);
  }
}

static std::strong_ordering CompareBuffers(const std::vector<uint32_t>& lhs,
                                           const std::vector<uint32_t>& rhs) {
  if (lhs.size() > rhs.size()) {
    return std::strong_ordering::greater;
  } else if (lhs.size() < rhs.size()) {
    return std::strong_ordering::less;
  }

  return std::lexicographical_compare_three_way(lhs.crbegin(), lhs.crend(),
                                                rhs.crbegin(), rhs.crend());
}