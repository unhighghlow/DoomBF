this test does not send any output since no flag is set
>> move to the first flag position
[ send 42 block
    <<[-] rewind to the position of an acc1 and reset it
    >[-]++++++ reset the acc2 then store 6 there
    [<+++++++>-] using a loop to store 6 * 7 in acc1
    <. send the number from acc1 to the output
    >>[-] return to the position of a flag and reset it
]> go to the next block flag
[ send 69 block
    <<<[-] rewind to the position of an acc1 and reset it
    >[-]++++++++++ reset the acc2 then store 10 there
    [<+++++++>-] using a loop to store 10 * 7 in acc1
    <-. decrement acc1 by one and send it to the output
    >>>[-] return to the position of a flag and reset it
]> go to the next block flag
