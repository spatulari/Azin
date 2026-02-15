# Variables

Variables store data.

In Azin, every variable must declare its type explicitly.

There is no dynamic typing.

---

## Basic Declaration

```azin
int x = 10;
```

Structure:

```
<type> <name> = <value>;
```

Example:

```azin
bool flag = true;
char c = 'A';
int number = 42;
```

---

## Declaration Without Initialization

Variables can be declared without an initial value.

```azin
int x;
```

⚠ Warning: The value is undefined until assigned.

You should initialize variables before using them.

---

## Assignment

Variables can be reassigned after declaration.

```azin
int x = 5;
x = 10;
```

Assignment rules:

- Types must match
- Some implicit conversions are allowed (e.g. `char` ↔ `int`)

Example:

```azin
char c;
c = 65;      // Allowed (within range)
```

---

## Array Variables

Currently, only `char` arrays are supported.

```azin
char buf[20];
```

Rules:

- Size must be a constant integer
- Only `char` arrays are supported
- Arrays cannot be resized
- Arrays decay to `char*` when passed to functions

Example:

```azin
char buf[20];
intToString@std(123, buf);
```

---

## Scope

Variables are scoped to their block.

```azin
int main()
{
    int x = 10;

    if (true)
    {
        int y = 20;
    }

    // y is NOT accessible here
}
```

Block scope rules:

- Variables only exist inside `{ }`
- Inner scopes can access outer scope variables
- Outer scopes cannot access inner scope variables

---

## Redeclaration

You cannot redeclare a variable in the same scope.

Invalid:

```azin
int x = 5;
int x = 10;  // Error
```

Valid (different scope):

```azin
int x = 5;

if (true)
{
    int x = 10;   // Allowed (shadows outer x)
}
```

---

## Type Safety

Azin enforces type correctness.

Invalid:

```azin
int x = true;      // Error
bool flag = 10;    // Error
```

Allowed:

```azin
char c = 5;
int x = 'A';
```

---

## Arrays and Indexing

You can access array elements using indexing.

```azin
char buf[10];
buf[0] = 'A';
```

Rules:

- Index must be `int`
- Indexing returns a `char`
- No bounds checking is performed

⚠ Out-of-bounds access is undefined behavior.

---

## Current Limitations

- Only `char` arrays are supported
- No pointer variables (`int* ptr = &x` not supported)
- No `const` keyword
- No `static` storage
- No struct variables (yet)

---

## Best Practices

- Initialize variables when possible
- Keep scope small
- Avoid shadowing unless necessary
- Use arrays for buffers only

---

## Philosophy

Variables in Azin are:

- Explicit
- Strict
- Predictable
- Close to the metal

No hidden behavior.

What you write is what the machine executes.
