# out

Outputs a null-terminated string to standard output.

---

## Declaration

```azin
nore out(char* msg);
```

---

## Description

`out` writes a string to file descriptor `1` (stdout).

Internally, it:

1. Calculates the string length using `strlen`
2. Calls the system-level `write` function

It does **not** append a newline automatically.

---

## Example

```azin
!use "std.az"

int main()
{
    out@std("Hello, World!");
    return 0;
}
```

Output:

```
Hello, World!
```

---

## How It Works Internally

Simplified implementation:

```azin
nore out(char* msg)
{
    int len = strlen(msg);
    write(1, msg, len);
}
```

The `write` function is declared as:

```azin
extern i64 write(int fd, char* msg, int count);
```

Where:

- `1` = stdout
- `msg` = pointer to characters
- `count` = number of bytes to write

---

## Important Notes

- The string **must** be null-terminated (`0`)
- No bounds checking is performed
- No formatting support
- No automatic newline

If you want a newline:

```azin
out@std("Hello\n");
```

---

## Usage With Buffers

```azin
char buf[20];
intToString@std(123, buf);
out@std(buf);
```

---

## Current Limitations

- Only supports `char*`
- No formatting (`%d`, `%s`, etc.)
- No stream abstraction
- No buffering layer

This is a minimal, low-level output function.

---

## Design Philosophy

`out` is intentionally simple.

It provides:

- Direct control
- No hidden behavior
- No runtime overhead
- No formatting engine

If you want advanced output features, build them on top of this.

Azin gives you primitives — not magic.
