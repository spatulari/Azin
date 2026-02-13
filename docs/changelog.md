# Change Log

---

## v0.2.0-alpha

### BIG CHANGE

I rewrote the whole code, from scratch, there is notably less feutures like no printing and etc, but i am working on it.
Instead of compiling to nasm, it compiled to C, so it means this is for windows and linux and any other OS.

### Feutures removed

- printing (aka out@io(...))
- `io` library, there is none

### New feutures

- int main function
- abillity to make functions with nore (which is the equivelant of void) and int
- functions with parameters
- calling functions
- if/else (No else if yet)

### Examples

```az
int sub(int a, int b)
{
    return a - b;
}

int main()
{
    int result = sub (8, 3);
    return result;
}
```

## v0.1.1-alpha

### New features

- comments
- bool variables
- can now include the io library

### Changes

- The `io` library must now be explicitly imported before use

### Examples

Now, instead of

```Azin
out@io("Hello World");
```

> This doesn't work anymore

this replaces it

```Azin
!use <io>  // this is a comment

out@io("Hello World");
```

> This prints Hello World

To include libraries, you do

```Azin
!use <...>
```


## v0.1.0-alpha

This is the first (pre)release of the langauge. It includes printing, variables and being able to return a number. That's it. 

### Examples

```Azin
int x = 5;

out@io(x);
```

> This prints x


```Azin
out@io("Hello World");
```

> This prints Hello World
