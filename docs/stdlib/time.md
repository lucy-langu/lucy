# `time` standard library — Lucy 2.0.2

Time/date/date-time objects, parsing, formatting, timestamps, and sleeping.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import time
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println time.now()
println time.today()
println time.format(time.timestamp())
```

## Exported module-level objects

`date`, `datetime`, `time`

## Module-level functions

| Function | Signature |
|---|---|
| `now` | `now()` |
| `today` | `today()` |
| `timestamp` | `timestamp()` |
| `parse` | `parse(text)` |
| `date` | `date(text)` |
| `format` | `format(value, pattern = "%Y-%m-%d %H:%M:%S")` |
| `_time_sleep` | `_time_sleep(milliseconds)` |
| `sleep` | `sleep(milliseconds)` |

## Classes and methods

### `Time`

| Method | Signature |
|---|---|
| `initialize` | `initialize(timestamp = nil)` |
| `now` | `now()` |
| `today` | `today()` |
| `strptime` | `strptime(text, format)` |
| `format` | `format(format_string)` |
| `parts` | `parts()` |
| `year` | `year()` |
| `month` | `month()` |
| `day` | `day()` |
| `hour` | `hour()` |
| `minute` | `minute()` |
| `second` | `second()` |
| `add` | `add(milliseconds)` |
| `subtract` | `subtract(milliseconds)` |
| `plus` | `plus(milliseconds)` |
| `minus` | `minus(milliseconds)` |
| `compare` | `compare(other)` |
| `succ` | `succ()` |
| `add_ms` | `add_ms(milliseconds)` |
| `subtract_ms` | `subtract_ms(milliseconds)` |

### `Date`

| Method | Signature |
|---|---|
| `initialize` | `initialize(timestamp = nil)` |
| `parse` | `parse(text)` |
| `strptime` | `strptime(text, format)` |
| `format` | `format(format_string = "%Y-%m-%d")` |
| `add` | `add(days)` |
| `subtract` | `subtract(days)` |
| `next_day` | `next_day()` |
| `prev_day` | `prev_day()` |
| `succ` | `succ()` |
| `shift_months` | `shift_months(months)` |
| `add_months` | `add_months(months)` |
| `subtract_months` | `subtract_months(months)` |
| `compare` | `compare(other)` |

### `DateTime`

| Method | Signature |
|---|---|
| `initialize` | `initialize(timestamp)` |
| `timestamp` | `timestamp()` |
| `format` | `format(pattern)` |
| `parts` | `parts()` |
| `year` | `year()` |
| `month` | `month()` |
| `day` | `day()` |
| `hour` | `hour()` |
| `minute` | `minute()` |
| `second` | `second()` |
| `weekday` | `weekday()` |
| `iso` | `iso()` |

### `DateTimeModule`

| Method | Signature |
|---|---|
| `now` | `now()` |
| `from_timestamp` | `from_timestamp(milliseconds)` |
| `format` | `format(milliseconds, pattern)` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/time.lucy`](../../stdlib/time.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
