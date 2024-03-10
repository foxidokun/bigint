#include <compare>
#include <cstdint>
#include <string_view>
#include <vector>

class BigInt {
  enum class Sign : int8_t { Negative, Zero, Positive };

 public:
  // default ctor
  BigInt() = default;

  // default copy
  BigInt(const BigInt& other) = default;
  BigInt& operator=(const BigInt& other) = default;

  // default move
  BigInt(BigInt&& other) = default;
  BigInt& operator=(BigInt&& other) = default;

  // constructing from other types
  explicit BigInt(int64_t);
  explicit BigInt(std::string_view);

  // Some convertions
  explicit operator bool() const {
    return sign_ != Sign::Zero;
  }

  // Math
  BigInt& operator+=(const BigInt& other);
  BigInt& operator-=(const BigInt& other);
  BigInt& operator*=(const BigInt& other);
  BigInt& operator/=(const BigInt& other);
  BigInt& operator%=(const BigInt& other);

  // Small math
  BigInt& operator++();
  BigInt& operator--();
  BigInt operator++(int);
  BigInt operator--(int);
  BigInt operator-();

  // Absurd & costly
  BigInt operator+() = delete;

  // Comparison
  std::strong_ordering operator<=>(const BigInt& other) const;
  bool operator==(const BigInt& other) const;

 private:
  BigInt(Sign sign, std::vector<uint64_t> digits)
      : sign_(sign),
        digits_(std::move(digits)) {
  }

  Sign sign_{Sign::Zero};
  std::vector<uint64_t> digits_;
};

static BigInt operator""_bi(unsigned long long val) {  // NOLINT
  return BigInt(val);
}

static BigInt operator""_bi(const char* val) {
  return BigInt(val);
}

static BigInt operator+(BigInt self, const BigInt& other) {
  self += other;
  return self;
}

static BigInt operator-(BigInt self, const BigInt& other) {
  self -= other;
  return self;
}

static BigInt operator*(BigInt self, const BigInt& other) {
  self *= other;
  return self;
}

static BigInt operator/(BigInt self, const BigInt& other) {
  self /= other;
  return self;
}

static BigInt operator%(BigInt self, const BigInt& other) {
  self %= other;
  return self;
}