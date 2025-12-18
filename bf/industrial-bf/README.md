# ibf
*The industrial brainfuck interpreter*
```
make ibf
ibf [<program.b>]
```

## Configuration
Configuration is done at compile-time in `evaluator/config.h`

- `PAGE_SIZE_POWER`: Sets the page size (2\*\*PAGE\_SIZE\_POWER)
- `CELL`: Configures the integer type that is used for the tape cells.

## Extra features

By default this interpreter conforms to the specification (as describled on [brainfuck.org](https://brainfuck.org/brainfuck.html)), but additional features can be enabled using compile-time flags:

### Debugger

Use the `DEBUGGER` option

Replaces the execution environment with a simple CLI debugger that allows you to step a single instruction or run the program until a breakpoint.

Adds a new instruction: `#` (breakpoint)

In the debugger UI, the following commands can be used:
```
? - display help
r - run the program until a breakpoint is reached
s - step one instruction forward
x - run until the current loop ends
n - run until the next sourcemap line
a - load an addrmap file
q - quit
```

If you input an empty line, the last command will be repeated

#### Addressmaps

An addressmap can be used to display the values of variables from specific tape locations. An addressmap line has the following format:

```
(mode)(addr)[(length)/(step)] (name)
```

Where `length` and `step` is optional

This address represents every `step`th element in the range of `addr`-`addr+length`

Example:
```
d0 address
x1 output
x2 scrap

s3[e] array
a3[e] array

a4[e/2] stepped_array
```

The modes are as follows:
- `x` -- Print in hexadecimal
- `d` -- Print in decimal
- `a` -- Print as an array
- `s` -- Print as strings

#### Sourcemaps

Every (non-whitespace) comment is assumed to annotate some code (or EOF). For example in the following program:
```
a
+ b
@
- c
-
d >+ e
<
x +
y +
-
z <
end
```

The comments are as follows:
```
[a
+ b
@]
[- c
-]
[d >+] [e
<]
[x +]
[y +
-]
[z <]
[end]
```

When executing, the comments are displayed and can be used as breakpoints with the `n` command.

Note: when reading comments, any continuous sequence of whitespace characters is replaced with a space, and whitespace at the beginning of the comment is removed

### Asserts

Use the `ASSERTS` option

Adds 2 new commands: `!` (assert value) and `@` (assert location). When executed, they check if the *current cell's value*/*data pointer* matches the expected value. If it does not, they interrupt execution and print an error message.

The expected value may be specified after the instruction in hexadecimal:
```
>-
@1
!ff
```

If the expected value isn't specified, it defaults to 0.

## Internal representation

The following commands are used by the interpreter internally, and can be viewed in the debugger:

```
+-------------------------------+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|===============================|
| + |VAL|   .   .   .   .   .   .
| - |VAL|   .   .   .   .   .   .
| < |VAL|   .   .   .   .   .   .
| > |VAL|   .   .   .   .   .   .
| / |VAL|   .   .   .   .   .   .
| ^ |OFFSET |VAL|   .   .   .   .
| [ | END INDEX                 |
| ] | BEGINNING INDEX           |
| ! | EXPECTED VALUE            |
| @ | EXPECTED VALUE            |
| 0 |   .   .   .   .   .   .   .
| . |   .   .   .   .   .   .   .
| , |   .   .   .   .   .   .   .
| # |   .   .   .   .   .   .   .
|\0 |   .   .   .   .   .   .   .
+-------------------------------+
```

# bld
*The ibf preloader*
```
make bld
./bld <input> [page_size_power=24]
```

Generates the pagefiles needed to preload *input* onto `ibf`'s tape. `page_size_power` must match the `PAGE_SIZE_POWER` configured in `ibf`
