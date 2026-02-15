# strlen

Returns the length of a null-terminated string.

---

## Signature

```azin
int strlen(char* s);
```

---

## Description

`strlen` counts characters in a string until it encounters the null terminator (`0`).

It does **not** allocate memory.  
It does **not** validate the pointer.  
It does **not** check bounds.

It simply walks memory forward until it finds `0`.

---

## Parameters

| Name | Type     | Description                     |
|------|----------|---------------------------------|
| `s`  | `char*`  | Pointer to null-terminated string |

---

## Returns

- `int` — number of characters before the null terminator

The null terminator itself is **not** included in the count.

---

## Example

```azin
char msg[6];

msg[0] = 'H';
msg[1] = 'e';
msg[2] = 'l';
msg[3] = 'l';
msg[4] = 'o';
msg[5] = 0;

int len = strlen@std(msg);
```

`len` will be:

```
5
```

---

## Implementation

The reference implementation:

```azin
int strlen(char* s)
{
    int i = 0;

    while (s[i] != 0)
    {
        i = i + 1;
    }

    return i;
}
```

---

## Important Notes

### ⚠ No Safety Checks

If:

- The pointer is invalid
- The string is not null-terminated
- The buffer is corrupted

This function will read past memory and cause undefined behavior.

There are no guards.

---

## Performance

- Time complexity: **O(n)**
- Memory usage: **O(1)**

It performs a simple linear scan.

---

## Design Philosophy

`strlen` exists because:

- Azin uses C-compatible null-terminated strings
- No runtime exists
- All string handling is manual
`strlen` assumes you know what you're doing.

---

## When to Use

Use `strlen` when:

- You need the size of a string before calling `write`
- You need bounds for loops
- You are manually managing buffers

Avoid using it in hot loops repeatedly on the same string — cache the length instead.
