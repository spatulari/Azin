# Standard Library (std)

The Azin Standard Library provides minimal, low-level utilities required for basic programs.

It is intentionally small, predictable, and close to the machine.

---

## Overview

The standard library is organized into modules:

```
std/
├── io/
│   └── out
└── utils/
    ├── intToString
    └── strlen
```

Each module must be imported using:

```azin
!use "std.az"
```

---

# Modules

## I/O

### [`out`](io/out.md)

Low-level console output.

```azin
nore out(char* msg);
```

Writes a null-terminated string to stdout using the `write` syscall.

No formatting.  
No buffering.  
No safety checks.

You control everything.

---

## Utilities

### [`intToString`](utils/intToString.md)

```azin
nore intToString(int x, char* buffer);
```

Converts an integer to a null-terminated ASCII string.

No heap allocation.  
You provide the buffer.

---

### [`strlen`](utils/strlen.md)

```azin
int strlen(char* s);
```

Returns the length of a null-terminated string.

Linear scan until `0`.

---

# Design Philosophy

The Azin standard library is:

- Minimal
- Explicit
- Non-magical
- Predictable

There is:

- No hidden allocation
- No runtime
- No formatting engine
- No safety rails

You are responsible for:

- Buffer sizes
- Valid pointers
- Memory correctness

This is intentional.

Azin is a systems-first language.

---

# Example Program

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

# Why So Small?

Azin does not try to compete with large runtime environments.

Instead:

- Core primitives are provided
- Higher-level abstractions are built by users
- You control complexity

This keeps the language:

- Lightweight
- Portable
- Bootstrappable
- Self-host friendly

---

# Future Expansion

Planned areas for growth:

- Memory utilities
- Safer string helpers
- Formatting layer
- Extended numeric conversions

All while preserving low-level control.

---

If you’re writing in Azin, you are close to the metal.

The standard library stays out of your way.
