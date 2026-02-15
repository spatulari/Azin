# Modules

Modules in Azin allow code to be organized across multiple files.

They are simple, explicit, and resolved at compile time.

There is no runtime module system.

---

## Importing a Module

To import another Azin file:

```azin
!use "std.az"
```

This tells the compiler:

- Load `std.az`
- Parse it
- Merge its functions into the program
- Apply name mangling to avoid conflicts

---

## Calling a Function From a Module

Functions from a module are called using:

```azin
functionName@module(...)
```

Example:

```azin
intToString@std(123, buf);
out@std(buf);
```

---

## Name Mangling

Internally, Azin mangles module functions to prevent collisions.

This:

```azin
intToString@std
```

Becomes:

```c
std__intToString
```

So two modules can define the same function name safely:

```
math.az → add@math
utils.az → add@utils
```

They compile into:

```
math__add
utils__add
```

No conflicts.

---

## What Gets Imported

When using `!use`:

- All functions in the file are available
- There is currently no symbol visibility system
- There is no `private` keyword
- Everything is exported

If you don’t want something accessible,
just don’t use it.

Simple.

---

## How Module Loading Works

The compiler:

1. Detects `!use`
2. Recursively loads the referenced file
3. Parses it
4. Merges declarations into the AST
5. Applies name mangling

Circular dependencies are not supported.

---

## Module Rules

- The path must be a string literal.
- The file must exist.
- Only `.az` files are supported.
- Functions must be declared normally.
- Modules are compile-time only.

---

## Example

### std.az

```azin
nore out(char* msg)
{
    write(1, msg, strlen(msg));
}
```

### main.az

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

---

## Design Philosophy

Azin modules are intentionally:

- Minimal
- Predictable
- Static
- Transparent

No:

- Dynamic linking
- Lazy loading
- Namespaces
- Package manager
- Versioning

Just files and mangled symbols.

---

## Future Possibilities

Planned improvements may include:

- Selective imports
- Private functions
- Better error messages
- Module search paths
- Header-style exports

But for now:

Modules are simple file inclusion with namespacing.

And that’s enough.
