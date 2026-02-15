# intToString

Converts an integer into a null-terminated string.

---

## Declaration

```azin
nore intToString(int x, char* buffer);
```

---

## Description

`intToString` converts an `int` value into its ASCII string representation.

The result is written into the provided `buffer`.

The buffer must:

- Be writable
- Be large enough to hold the result
- Allow space for the null terminator (`0`)

---

## Parameters

| Name     | Type     | Description                          |
|----------|----------|--------------------------------------|
| `x`      | `int`    | The integer to convert               |
| `buffer` | `char*`  | Destination character buffer         |

---

## Example

```azin
!use "std.az"

int main()
{
    char buf[20];

    intToString@std(123, buf);
    out@std(buf);

    return 0;
}
```

Output:

```
123
```

---

## Negative Numbers

Negative values are supported:

```azin
intToString@std(-42, buf);
```

Produces:

```
-42
```

---

## Zero Handling

Zero is handled explicitly:

```azin
intToString@std(0, buf);
```

Produces:

```
0
```

---

## Internal Behavior (Simplified)

1. Detect if number is zero
2. Detect if number is negative
3. Extract digits using modulo (`% 10`)
4. Convert digit to ASCII using `'0'`
5. Reverse the buffer
6. Null-terminate

Core logic conceptually:

```azin
digit = x % 10
buffer[i] = digit + '0'
```

Digits are generated in reverse order and then swapped into correct order.

---

## Buffer Size Requirements

You are responsible for providing enough space.

Recommended size for 32-bit integers:

```azin
char buf[12];
```

Why 12?

- 10 digits max
- 1 optional `-`
- 1 null terminator

---

## Important Warning

```azin
x = -x;
```

This causes **undefined behavior** if `x == INT_MIN`.

Why?

Because `INT_MIN` cannot be represented as a positive `int`.

This is a known limitation in the current implementation.

---

## Safety Notes

- No bounds checking
- No overflow detection
- No automatic allocation
- Caller owns the buffer

If buffer is too small → memory corruption.

Be responsible.

---

## Performance Characteristics

- O(d) where d = number of digits
- No dynamic allocation
- Pure arithmetic operations
- No recursion

Extremely lightweight.

---

## Design Philosophy

`intToString` is intentionally low-level.

It does:

- Deterministic conversion
- No formatting
- No heap usage
- No hidden behavior