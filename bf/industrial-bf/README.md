# ibf
```
make ibf
ibf [<program.b>]
```

## Configuration
See `src/main.c`#6

- `CELL`: Configures the integer type that is used for the tape cells.

# bfm
```
make bfm
bfm <input.b> [<output.bfm>]
```

## The `.bfm` (BrainF macros) format

In short: Each command is now two bytes with the second byte representing the number of repetitions minus one. `[`, `]`, `.` and `,` cannot be repeated
