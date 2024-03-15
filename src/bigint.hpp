#include <compare>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string_view>
#include <vector>

class BigInt {
   public:
  enum class Sign : int8_t { Negative, Zero, Positive };

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
  explicit operator bool() const { return sign_ != Sign::Zero; }

  // Math
  BigInt& operator+=(const BigInt& other);
  BigInt& operator-=(const BigInt& other);
  BigInt& operator*=(const BigInt& other);
  BigInt& operator/=(const BigInt& other);
  BigInt& operator%=(const BigInt& other);

  // Int Operations
  BigInt& operator+=(int32_t other);
  BigInt& operator-=(int32_t other);
  BigInt& operator*=(int32_t other);

  // Small math
  BigInt& operator++();
  BigInt& operator--();
  BigInt operator++(int);
  BigInt operator--(int);
  BigInt operator-() const;

  // Absurd & costly
  BigInt operator+() = delete;

  // Comparison
  std::strong_ordering operator<=>(const BigInt& other) const;
  bool operator==(const BigInt& other) const;

  void LeftShift(uint32_t digit_num);

   private:
  BigInt(Sign sign, std::vector<uint32_t> digits)
      : sign_(sign), digits_(std::move(digits)) {}

  static Sign OppositeSign(Sign);
  bool IsSameSignAs(int32_t);

  BigInt GetDivDigit(const BigInt& other) const;
  uint64_t GetHead(int64_t start_index = 0) const;

  friend std::ostream& operator<<(std::ostream& stream, BigInt val);
  friend Sign operator*(const Sign& lhs, const Sign& rhs);

  Sign sign_{Sign::Zero};
  std::vector<uint32_t> digits_;
};

static BigInt operator""_bi(const char* val, std::size_t len) {
  return BigInt(std::string_view(val, len));
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

static BigInt operator+(BigInt self, int32_t other) {
  self += other;
  return self;
}

static BigInt operator-(BigInt self, int32_t other) {
  self -= other;
  return self;
}

static BigInt operator*(BigInt self, int32_t other) {
  self *= other;
  return self;
}

static BigInt operator+(int32_t other, BigInt self) {
  self += other;
  return self;
}

static BigInt operator-(int32_t other, BigInt self) {
  self -= other;
  return self;
}

static BigInt operator*(int32_t other, BigInt self) {
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

std::istream& operator>>(std::istream& stream, BigInt& val);