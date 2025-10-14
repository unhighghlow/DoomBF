Memory efficient addressing

Algorithm:
* Rotate the array `address` times clockwise
* Read the first element
* Rotate the array back

Memory layout:
00 address (consumed)
01 output  (created)
02 scrap
03 array

Reserve address and output
>>

Load the array into memory (and reserve scrap)
 0 H>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 1 e>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 2 l>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 3 l>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 4 o>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 5  >++++++++++++++++++++++++++++++++
 6 w>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 7 o>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 8 r>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 9 l>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
10 d>++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
11 !>+++++++++++++++++++++++++++++++++
12 2>++
13 3>+++ (outside of the array)

Go to address
[<]<<

Set the address
6 ++++++

BEGIN ALGORITHM

Copy address to output through scrap

  [->>+<<]
  >>[-<+<+>>]<<

For the first rotation use the output cell for the address
Move to output

  >

Move the whole array backwards
[
  Move to array

    >>

  Move first element of the array to scrap

    [ -<+> ]

  Move the elements of the array backwards

    >[-<+>]
  
  The above is repeated a total of `max_addr` times
    >[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]>[-<+>]

  We're at the end of the array
  Move to the start
  "move left" repeated `max_addr` times

    <<<<<<<<<<<<

  Move to scrap

    <
  
  Move scrap to the end of the array

    [ - > >>>>>>>>>>>> + < <<<<<<<<<<<< ]

  Move to address
    
    <

  Decrement the address

    -
]

Output (which was used as address) is now empty
Move to address

  <

Move first element to scrap and output

  >>>[-<<+>+>]<

We're at scrap
Move scrap to the first element

  [->+<]<<

We're at address
Move the whole array forwards

[
  Go to the start of the array

    >>>

  Go to the end of the array
  "move left" repeated `max_addr` times

    >>>>>>>>>>>>

  Move the last element to scrap

    [
      <<<<<<<<<<<< 
      < +
      >>>>>>>>>>>> 
      > -
    ]

  Move elements of the array forwards

    <[->+<]

  The above is repeated a total of `max_addr` times

    <[->+<]<[->+<]<[->+<]<[->+<]<[->+<]<[->+<]<[->+<]<[->+<]<[->+<]<[->+<]<[->+<]

  Move scrap to the start of the array

    <[->+<]

  We're at scrap
  Move to address

    <<

  Decrement address

    -
]

Output

  >.
