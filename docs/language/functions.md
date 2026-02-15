# Functions

Functions are the primary way to structure reusable logic in Azin.

A function has:
- A return type
- A name
- Zero or more parameters
- A body

---

## Basic Syntax

```azin
return_type function_name(type param)
{
    // body
}
```

Example:

```azin
int add(int a, int b)
{
    return a + b;
}
```

---

## Calling a Function

```azin
int result = add(5, 3);
```

Arguments must:
- Match the parameter count
- Be type-compatible

---

## Return Types

Every function must declare a return type.

### Returning a Value

```azin
int square(int x)
{
    return x * x;
}
```

If the return type is not `nore`, a `return` statement is required.

---

## `nore` Functions

Use `nore` for functions that do not return a value.

```azin
nore greet()
{
    out("hello");
}
```

Rules:

- `nore` functions cannot return a value
- They may use `return;` with no expression
- Missing `return` is allowed

---

## Parameters

Parameters are passed by value.

```azin
int multiply(int x, int y)
{
    return x * y;
}
```

---

## Arrays as Parameters

Arrays decay to pointers when passed to functions.

```azin
nore print(char* buffer)
{
    out(buffer);
}
```

You **cannot** declare parameters as:

```azin
char[] buffer   // ❌ not allowed
```

Always use pointer form:

```azin
char* buffer    // ✅ correct
```

---

## Module Functions

When using modules, functions are referenced with `@`.

Example:

```azin
intToString@std(123, buffer);
```

This maps internally to:

```c
std__intToString(...)
```

---

## Function Overloading

Not supported.

Function names must be unique within a module.

---

## Restrictions

- No default parameters
- No variadic functions
- No recursion limits enforced by language (stack dependent)

---

## Design Philosophy

Azin keeps functions:

- Explicit
- Low-level
- Predictable
- Close to C semantics

No hidden magic.
No runtime abstraction.
Just structured control over execution.
