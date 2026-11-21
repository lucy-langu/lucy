# Lucy `math` library

Math helpers written in pure Lucy: rounding, powers, logs, trigonometry, integer tools, statistics, random numbers and small vector helpers.

```lucy
import math

print math.sqrt(16)
print math.max(3, 8)
print math.mean([1, 2, 3, 4])
```

---

# Conventions

- **Angles are in radians.** Use `to_radians` and `to_degrees` to convert.
- **Bad input returns `nan`** instead of stopping the program. Examples: `sqrt(-1)`, `ln(-5)`, `mean([])`.
- **Check for `nan` with `is_nan`**, never with `==`. `nan == nan` is false.
- **Precision:** results are accurate to about 12 digits, not always bit-identical to C++ `<cmath>`.
- **Sequences are never modified.** Functions like `sorted` and `shuffle` return a new array.

---

# Constants

| Name | Value | Meaning |
| --- | --- | --- |
| `pi` | 3.14159... | π |
| `tau` | 6.28318... | 2π |
| `e` | 2.71828... | Euler's number |
| `ln2` | 0.69314... | ln(2) |
| `ln10` | 2.30258... | ln(10) |
| `log2e` | 1.44269... | log2(e) |
| `log10e` | 0.43429... | log10(e) |
| `sqrt2` | 1.41421... | √2 |
| `sqrt1_2` | 0.70710... | 1/√2 |
| `inf` | | positive infinity |
| `neg_inf` | | negative infinity |
| `nan` | | not a number |

---

# Basic

| Function | Description |
| --- | --- |
| `abs(x)` | Absolute value. |
| `sign(x)` | `-1`, `0` or `1` (`nan` for `nan`). |
| `min(a, b)` | Smaller of two numbers. |
| `max(a, b)` | Larger of two numbers. |
| `clamp(x, low, high)` | Limits `x` to the range `low..high`. |
| `floor(x)` | Rounds down: `floor(-2.5)` is `-3`. |
| `ceil(x)` | Rounds up: `ceil(-2.5)` is `-2`. |
| `round(x)` | Nearest whole number, halves go away from zero: `round(2.5)` is `3`, `round(-2.5)` is `-3`. |
| `trunc(x)` | Drops the fractional part: `trunc(-2.7)` is `-2`. |
| `fract(x)` | `x - floor(x)`, so `fract(-1.25)` is `0.75`. |
| `mod(a, b)` | Remainder with the sign of `b` (like Ruby `%`). `mod(-7, 3)` is `2`. |
| `rem(a, b)` | Remainder with the sign of `a` (like C++ `fmod`). `rem(-7, 3)` is `-1`. |

`mod` and `rem` return `nan` when `b` is `0`.

---

# Powers, roots and exponentials

| Function | Description |
| --- | --- |
| `pow(base, power)` | `base` raised to `power`. Integer powers use `pow_int`, other powers use `exp` and `ln`. |
| `pow_int(base, power)` | Fast power for integer exponents. Negative exponents give `1 / base^n`. |
| `sqrt(x)` | Square root. `nan` for negative input. |
| `cbrt(x)` | Cube root. Works for negative numbers. |
| `hypot(x, y)` | `sqrt(x*x + y*y)`. |
| `exp(x)` | e raised to `x`. |
| `exp2(x)` | 2 raised to `x`. |
| `expm1(x)` | `exp(x) - 1`, accurate for very small `x`. |

```lucy
print math.pow(2, 10)     # 1024
print math.pow(2, 0.5)    # 1.41421...
print math.pow(2, -2)     # 0.25
print math.cbrt(-27)      # -3
```

`pow(0, negative)` gives `inf`. `pow(negative, fraction)` gives `nan`.

---

# Logarithms

| Function | Description |
| --- | --- |
| `ln(x)` | Natural logarithm. `ln(0)` is `neg_inf`, negative input gives `nan`. |
| `log(x, base)` | Logarithm of `x` in any base. |
| `log2(x)` | Base-2 logarithm. |
| `log10(x)` | Base-10 logarithm. |
| `log1p(x)` | `ln(1 + x)`, accurate for very small `x`. |

```lucy
print math.log(8, 2)      # 3
print math.log10(1000)    # 3
```

---

# Trigonometry

| Function | Description |
| --- | --- |
| `sin(x)`, `cos(x)`, `tan(x)` | Standard trigonometric functions (radians). |
| `arcsin(x)`, `arccos(x)` | Inverse sine and cosine. Input must be in `-1..1`, otherwise `nan`. |
| `arctan(x)` | Inverse tangent, result in `-pi/2..pi/2`. |
| `arctan2(y, x)` | Angle of the point `(x, y)`, result in `-pi..pi`. `arctan2(0, 0)` is `0`. |
| `sinh(x)`, `cosh(x)`, `tanh(x)` | Hyperbolic functions. |
| `arcsinh(x)`, `arccosh(x)` | Inverse hyperbolic functions. `arccosh` needs `x >= 1`. |
| `to_radians(deg)` | Degrees to radians. |
| `to_degrees(rad)` | Radians to degrees. |

C++ style aliases are also available: `atan`, `asin`, `acos`, `atan2`, `asinh`, `acosh`.

```lucy
angle = math.to_radians(90)
print math.sin(angle)                 # 1
print math.to_degrees(math.arctan2(1, 1))   # 45
```

---

# Checks

| Function | Returns `true` when |
| --- | --- |
| `is_nan(x)` | `x` is `nan`. |
| `is_inf(x)` | `x` is `inf` or `neg_inf`. |
| `is_finite(x)` | `x` is a normal number (not `nan`, not infinite). |
| `is_integer(x)` | `x` is finite and has no fractional part. |
| `is_normal(x)` | `x` is finite, not zero and not subnormal. |

---

# Integer helpers

| Function | Description |
| --- | --- |
| `gcd(a, b)` | Greatest common divisor (always positive). |
| `lcm(a, b)` | Least common multiple. `0` if either number is `0`. |
| `factorial(n)` | `n!`. `nan` for negative `n`. |
| `is_prime(n)` | `true` if `n` is prime. |
| `next_prime(n)` | Smallest prime greater than `n`. |
| `comb(n, k)` | Number of ways to choose `k` from `n`. |
| `perm(n, k)` | Number of ordered arrangements of `k` from `n`. |

```lucy
print math.gcd(12, 18)      # 6
print math.lcm(4, 6)        # 12
print math.comb(5, 2)       # 10
print math.next_prime(13)   # 17
```

Large results (for example `factorial(25)`) can overflow the integer type.

---

# Statistics

All of these take an array. Empty arrays return `nan`.

| Function | Description |
| --- | --- |
| `sum(list)` | Sum of all items. |
| `mean(list)` | Average. |
| `median(list)` | Middle value (average of the two middle values for even length). |
| `min_of(list)` | Smallest item. |
| `max_of(list)` | Largest item. |
| `variance(list)` | Population variance (divides by `n`). |
| `stddev(list)` | Population standard deviation. |
| `sorted(list)` | New array with the items in ascending order. |

```lucy
scores = [5, 3, 9, 1, 7]

print math.mean(scores)      # 5
print math.median(scores)    # 5
print math.max_of(scores)    # 9
print math.sorted(scores)    # [1, 3, 5, 7, 9]
```

`min(a, b)` and `max(a, b)` are for two numbers. For arrays use `min_of` and `max_of`.

---

# Random numbers

A simple seeded generator. Good for games and tests, **not for security**.

| Function | Description |
| --- | --- |
| `seed(value)` | Sets the seed. The same seed always gives the same sequence. |
| `random()` | Float in `[0, 1)`. |
| `rand_int(low, high)` | Integer from `low` to `high`, both included. |
| `rand_float(low, high)` | Float in `[low, high)`. |
| `choice(list)` | Random item from an array (`nan` if empty). |
| `shuffle(list)` | New array with the items in random order. |

```lucy
math.seed(42)
print math.rand_int(1, 6)       # dice roll
print math.choice(["a", "b", "c"])
```

---

# Geometry and vectors

Vectors are plain arrays like `[x, y, z]`.

| Function | Description |
| --- | --- |
| `distance(x1, y1, x2, y2)` | Distance between two 2D points. |
| `lerp(a, b, t)` | Point between `a` and `b`. `t = 0` gives `a`, `t = 1` gives `b`. |
| `inverse_lerp(a, b, x)` | Reverse of `lerp`: where `x` sits between `a` and `b`. |
| `map_range(x, in_min, in_max, out_min, out_max)` | Converts `x` from one range to another. |
| `dot(v1, v2)` | Dot product. `nan` if the lengths differ. |
| `cross(v1, v2)` | Cross product of two 3D vectors. |
| `length(v)` | Length (magnitude) of a vector. |
| `normalize(v)` | Vector with the same direction and length 1. |

```lucy
print math.distance(0, 0, 3, 4)               # 5
print math.map_range(5, 0, 10, 0, 100)        # 50
print math.cross([1, 0, 0], [0, 1, 0])        # [0, 0, 1]
print math.normalize([3, 4])                  # [0.6, 0.8]
```

---

# Notes for contributors

- Every method is called through `self.` inside the `Math` class. Constants are used by their bare name.
- New functions should follow the same rules: return `nan` for invalid input and never modify the arrays they receive.
- The algorithms are pure Lucy (Newton's method and Taylor series with range reduction), so results can differ from the host C library in the last digits.


---
 
# Author
 
Developed by **Abolfazlilka**.