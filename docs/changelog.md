# Change Log

---

## v0.3.0-alpha

## New feutures

- user created libraries
- printing 
- std library (has to be downloaded manually)
- =, ==, >=, <=, >, < operators
- char* and char array which are the same undercover

### Examples

main.az:

```az
!use "std.az"

int main()
{
    bool verified = false;
    int age = 19;
    char buf[20];

    intToString@std(age, buf);
    out@std("Age: ");
    out@std(buf);

    if (age >= 18)
    {
        verified = true;
    }
    else
    {
        verified = false;
    }

    if (verified)
    {
        out@std("\nVerified\n");
    }
    else
    {
        out@std("\nNot verified\n");
    }

    return 0;
}
```

std.az:

```az
extern i64 write(int fd, char* msg, int count); 

nore intToString(int x, char* buffer)
{
    int i = 0;
    bool isNegative = false;

    if (x == 0)
    {
        buffer[i] = '0';
        i = i + 1;
        buffer[i] = 0;
        return;
    }

    if (x < 0)
    {
        isNegative = true;
        x = -x;   // WARNING: UB on INT_MIN
    }

    while (x > 0)
    {
        int digit = x % 10;
        buffer[i] = digit + '0';
        i = i + 1;
        x = x / 10;
    }

    if (isNegative)
    {
        buffer[i] = '-';
        i = i + 1;
    }

    buffer[i] = 0;

    int start = 0;
    int end = i - 1;

    while (start < end)
    {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;

        start = start + 1;
        end = end - 1;
    }
}

int strlen(char* s) {
    int i = 0;

    while (s[i] != 0) {
        i = i + 1;
    }

    return i;
}

nore out(char* msg)
{
    int len = strlen(msg);
    write(1, msg, len);
}
```

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

> This returns 5, which you can check by echo %ERRORLEVEL% on windows and echo $? on linux

## v0.1.1-alpha

### New feutures

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
