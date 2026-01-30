# Change Log

---

## v0.1.1-alpha

### New features:

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

