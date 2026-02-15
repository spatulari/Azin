# The Azin Language

Azin is a low-level, statically typed programming language designed for systems programming and compiler experimentation.

It is:

- Explicit
- Minimal
- Deterministic
- Close to C semantics
- Designed for learning how languages work internally

---

## Language Philosophy

Azin avoids:

- Hidden runtime behavior
- Implicit memory management
- Complex abstractions
- Over-engineered features

If something happens, you wrote it.

If it breaks, it’s your fault.

And that’s the point.

---

## Core Concepts

Azin is built around:

- Functions
- Static types
- Manual memory handling
- Simple control flow
- Module-based code organization

There are no:

- Classes
- Generics
- Exceptions
- Macros
- Overloading
- Automatic heap allocation

Everything is explicit.

---

## Type System

Azin has a simple but strict type system.

Built-in types include:

- `int`
- `char`
- `bool`
- `nore`
- Fixed-width integers (`i8`, `i16`, `i32`, `i64`)
- Unsigned variants (`u8`, `u16`, `u32`, `u64`)


Arrays are supported (currently for `char`):

```azin
char buf[20];
```

Type compatibility rules are enforced during semantic analysis.

---

## Memory Model

- No garbage collection
- No automatic heap allocation
- Stack-based local variables
- Pointer arithmetic supported
- Array indexing supported

The language aims to behave predictably when compiled to C.

---

## Modules

Azin uses a simple module system.

Example:

```azin
!use "std.az"
```

Function calls from modules use:

```azin
functionName@module(...)
```

Internally mangled to:

```c
module__functionName(...)
```

---

## Compilation Pipeline

Azin is compiled through:

1. Lexical Analysis
2. Parsing
3. AST Construction
4. Semantic Analysis
5. C Code Generation
6. Native Compilation via C compiler

This makes Azin:

- Portable
- Easy to debug
- Transparent in behavior

---

## Current Limitations

Azin is experimental.

- The standard library is small
- Type system is minimal
- Error messages are basic
- No optimizer (yet)

The focus is learning and building, not stability.

---

## Next Steps

Explore:

- [Variables](variables.md)
- [Types](types.md)
- [Functions](functions.md)
- [Control Flow](control-flow.md)
- [Modules](modules.md)

---

Azin is not trying to compete.

It is trying to teach.

And to build something from zero.
