Time efficient addressing

Memory layout:
00 address (consumed)
01 scrap
02 output  (created)

03 ascrap
04 array(0)
05 ascrap
06 array(1)
and so on 

Algorithm:
* Fill the first `address` scrap bytes
* Use them to glide move the correct byte into scrap
* Move address back to the correct byte_ restoring the array

Reserve address and scrap
>>

Load the array into memory (and reserve output)
 0 H>>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 1 e>>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 2 l>>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 3 l>>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 4 o>>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 5  >>++++++++++++++++++++++++++++++++
 6 w>>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 7 o>>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 8 r>>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 9 l>>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
10 d>>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
11 !>>+++++++++++++++++++++++++++++++++
12 2>>++

Go to address
[<<]<<

Set the address
6 ++++++

BEGIN ALGORITHM

Increment the address

  +

Move address to ascrap(0)

  [->>>+<<<]

Move to ascrap(0)

  >>>

Create a glider chain

  [
    [->>+<<]
    >>-<<+>>
  ]
  <

We're at the correct byte now
Glide move the correct byte into address and output

[
  Decrement the correct byte

    -

  Go to the glide chain

    <

  Glide to the start

   [<<]

  We're at scrap now
  Move value to address and output

    <+>>+<

  Go to the glide chain

    >>

  Glide to the end

    [>>]

  Go to the correct byte

    <
]

Go to the glide chain

  <

Glide to the start

  [<<]

Go to output

  <

Glide move output to the correct byte
[
  Decrement output

    -

  Go to the glide chain

    >>>

  Glide to the end

    [>>]

  Go to the correct byte

    <

  Move the value

    +

  Go to the glide chain

    <

  Glide to the start

    [<<]

  Go to output

    <
]

Go to the glide chain

  >>>

Clean up the glide chain
  Glide to the end

    [>>]

  Go to the glide chain

    <<

  Glide to the start with cleanup

    [-<<]

Go to address

  <

Output

  >>.
