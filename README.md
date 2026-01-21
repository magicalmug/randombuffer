# randombuffer

A series of tests to determine cache speeds with some amount of precision

test1: random swap
fills various size buffers with 64bit integers, then swaps a random one with another 64 bit integer. 

test2: bucket sort
fills the various caches with 64 bit integers, then does an in-place bucket sort

--- Compile ---

gcc -o combine combine.c
