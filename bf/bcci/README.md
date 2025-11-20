# bcci
*Unreasonably restrictive, score-computing interpreter*
```
make
./bcci <orogram.b> [cpoulrt]
```

This interpreter uses byte cells, and gives an error on overflow or
underflow, or if the pointer is moved out of the array (whose size
is defined in ARRAYSIZE). It translates newlines, and leaves cell
values unchanged on end-of-file. It also reports the pointer's final
location and the final values of all cells visited during execution,
plus the number of brainfuck commands in the program, the number of
commands executed (using Faase's [] semantics), and the amount of
memory used, plus a composite score which is the product of these
last three numbers.
The pseudo-command '#' will zero all three numbers; this is to avoid
scoring any framework which is needed to set things up before the
execution of the component being tested, in the competition.

## Errors
You can specify which errors you want to disable in the second argument

Certain errors can be disabled:
- `o`: Overflow
- `u`: Underflow
- `r`: Too far right
- `t`: Command count overflow

Certain errors can, but shouldn't be disabled
- `c`: Unmatched ']'
- `p`: Unmatched '['
- `l`: Too far left

Certain errors can't be disabled
- `f`: I need a program filename
- `m`: Too many arguments
- `e`: Can't open that file
