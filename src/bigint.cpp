#include "bigint.hpp"
#include <algorithm>
#include <compare>
#include <cstdint>

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

  digits_.emplace_back(val);
}

BigInt::BigInt(std::string_view decimal_input) {
  __int128_t tmp = 0;
  constexpr __uint128_t kBase = UINT64_MAX;

  auto it = decimal_input.begin();
  if (*it == '-') {
    sign_ = Sign::Negative;
    ++it;
  }

  for (auto end = decimal_input.end(); it < end; ++it) {
    tmp = 10 * tmp + (*it - '0');

    // Improvment: Can be unrolled, do not check every digit
    if (tmp > kBase) {
      digits_.emplace_back(tmp % kBase);
      tmp >>= 64;
    }
  }

  if (tmp != 0) {
    digits_.emplace_back(tmp);
  }

  // sign correcting
  if (sign_ != Sign::Negative && !digits_.empty()) {
    sign_ = Sign::Positive;
  } else if (sign_ == Sign::Negative && digits_.empty()) {
    sign_ = Sign::Zero;
  }
}

BigInt& BigInt::operator+=(const BigInt& other) {
  /*Not implemented*/
  return *this;
}

BigInt& BigInt::operator-=(const BigInt& other) {
  /*Not implemented*/
  return *this;
}

BigInt& BigInt::operator*=(const BigInt& other) {
  /*Not implemented*/
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

BigInt& BigInt::operator++() {
  /*Not implemented*/
  return *this;
}

BigInt& BigInt::operator--() {
  /*Not implemented*/
  return *this;
}

BigInt BigInt::operator++(int) {
  /*Not implemented*/
  return *this;
}

BigInt BigInt::operator--(int) {
  /*Not implemented*/
  return *this;
}

BigInt BigInt::operator-() {
  /*Not implemented*/
  return *this;
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