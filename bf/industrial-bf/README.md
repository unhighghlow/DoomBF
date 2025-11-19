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
