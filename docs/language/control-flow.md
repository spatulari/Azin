# Control Flow

Control flow statements allow Azin programs to make decisions and repeat code execution.

Azin keeps control flow minimal, predictable, and close to low-level semantics.

---

## `if` Statement

The `if` statement executes a block only if the condition is `true`.

### Syntax

```azin
if (condition)
{
    // code
}
```

### Example

```azin
int x = 5;

if (x > 0)
{
    out("positive");
}
```

---

## `if-else`

Use `else` to execute an alternative block.

```azin
if (x > 0)
{
    out("positive");
}
else
{
    out("non-positive");
}
```

---

## `while` Loop

The `while` loop repeats execution while the condition is `true`.

### Syntax

```azin
while (condition)
{
    // code
}
```

### Example

```azin
int i = 0;

while (i < 5)
{
    out("looping");
    i = i + 1;
}
```

---

## Comparison Operators

Comparison operators return a `bool`.

| Operator | Meaning              |
|----------|---------------------|
| `==`     | Equal               |
| `!=`     | Not equal           |
| `<`      | Less than           |
| `>`      | Greater than        |
| `<=`     | Less than or equal  |
| `>=`     | Greater than or equal |

---

## Logical Type

Azin includes a built-in `bool` type.

Boolean values:

```azin
true
false
```

Conditions in `if` and `while` **must** evaluate to `bool`.

---

## Important Notes

- Parentheses around conditions are required.
- Braces `{}` are required for blocks.
- No implicit truthy/falsy behavior — only `bool` is allowed.

This keeps control flow explicit and predictable.
