# Types

Azin is a statically typed language.

Every variable and function must declare its type explicitly.

Type checking happens at compile time.

---

## Primitive Types

### Integer

```azin
int x = 10;
```

- Default signed integer type
- Used for arithmetic
- Used for indexing arrays

---

### Boolean

```azin
bool flag = true;
```

Possible values:

```
true
false
```

Used in:

- `if`
- `while`
- Comparisons

---

### Character

```azin
char c = 'A';
```

- Represents a single byte
- Can store ASCII values
- Compatible with `int` in arithmetic

Example:

```azin
char digit = 5 + '0';
```

---

## Fixed-Width Integers

Azin supports explicit integer sizes.

### Signed

```
i8
i16
i32
i64
```

Example:

```azin
i64 big = 123456789;
```

---

### Unsigned

```
u8
u16
u32
u64
```

Example:

```azin
u32 value = 42;
```

---

## Arrays

Currently, only `char` arrays are supported.

```azin
char buf[20];
```

Rules:

- Must specify size
- Only `char` arrays are allowed
- Arrays decay to `char*` when passed to functions

Example:

```azin
intToString@std(123, buf);
```

---

## Pointers

Basic pointer types exist in function parameters:

```azin
char* buffer
```

Used for:

- Strings
- Buffer passing

However:

- Pointer arithmetic is not supported
- `&` (address-of) is not supported
- `*ptr = value` dereferencing is not supported
- You cannot declare raw pointers like:

```azin
int* ptr = &x;   // Not supported
```

Pointer support is limited and controlled.

Full pointer support is planned for future versions.

---

## String Literals

String literals are automatically treated as `char*`.

```azin
out@std("Hello");
```

Internally:

- Stored as `char*`
- Null-terminated

---

## Type Compatibility

Some implicit conversions are allowed:

### Allowed

- `char` → `int`
- `int` → `char` (restricted)
- `int` → `i64`
- `int` → `u64`
- `char[]` → `char*` (array decay in function calls)

### Not Allowed

- `int` → `bool`
- `bool` → `int`
- Any unrelated type conversions
- Pointer conversions

Azin prefers safety over silent casting.

---

## nore Type

Used for functions that return nothing.

```azin
nore out(char* msg)
```

Equivalent to `void` in C.

Rules:

- Cannot return a value
- `return;` is allowed
- Must not return data

---

## Type Rules Summary

- All variables must declare a type
- Types are checked at compile time
- No implicit dynamic typing
- No generics
- No unions
- No structs (yet)

---

## Philosophy

Azin types are:

- Explicit
- Predictable
- C-like
- Minimal

No magic.

If it compiles, you know exactly what the machine will do.
