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

#### Addressmaps

An addressmap can be used to display the values of variables from specific tape locations. An addressmap line has the following format:

```
(mode)(addr)[(length)] (name)
```

Where `length` is optional

Example:
```
d0 address
x1 output
x2 scrap

s3[e] array
a3[e] array
```

The modes are as follows:
- `x` -- Print in hexadecimal
- `d` -- Print in decimal
- `a` -- Print as an array
- `s` -- Print as strings

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

# bld
*The ibf preloader*
```
make bld
./bld <input> [page_size_power=24]
```

Generates the pagefiles needed to preload *input* onto `ibf`'s tape. `page_size_power` must match the `PAGE_SIZE_POWER` configured in `ibf`
