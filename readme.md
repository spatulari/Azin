# Azin

Azin is an experimental programming language and compiler written in C++.  
The project is built from scratch with the goal of understanding how real-world compilers work at a low level.

This repository focuses on the complete compilation pipeline, including lexical analysis, parsing, AST construction, and basic code generation.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)

---

## Overview

Azin is primarily an educational project.  
It is designed to explore compiler internals rather than to be a production-ready language.

The compiler is implemented in modern C++ and follows a modular architecture, separating each compilation stage into its own component.

---

## Features

- Custom programming language syntax (`.az`)
- Lexer for tokenization
- Parser for syntax analysis
- Abstract Syntax Tree (AST)
- Basic code generation
- Modular and readable C++ codebase
- Simple command-line usage
- string printing
- int and bool variables
- variable printing
- basic errors on variables (e.g. bool x = 56; -> cant assign non-bool to bool)

---

## Syntax

```
int x = 5;
out@io(x);
```
This prints the variable x, variable decleration is like C++, it has only int and bool data types for now, and no int main() yet.
It can also print strings

```
out@io("Hello World");
```

---

### VSCode extension 

I have made a [VSCode extension](https://github.com/Alex12-Git200/Azin-Language-Support-VSCode) for syntax highlighting (soon intellisense)

---

## How to run it

This is only for linux.

Debian/Ubuntu:
```
sudo apt install g++ nasm gcc
```
This installs the dependencies.


Run this from the project root to build the compiler (or download a precompiled binary from the Releases page):
```
./build.sh 
```

Now, you have 'azc'. To use it, you run:
```
./azc yourfile.az
```

Note: there is no system wide compiler or stuff like that, later on it will.

