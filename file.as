MAIN: mov M1[r2][r7],LENGTH
 add r2,STR
LOOP: jmp END
 prn #-5
mcro mc_a
mov M1[r3][r3],r3
 bne LOOP
mcroend

mcro mc_b
prn #-5
 bne LOOP
mcroend

 sub r1, r4
 inc K  
mc_a 

mc_b
END: stop
STR: .string "abcdef"
LENGTH: .data 6,-9,15
K: .data 22
M1: .mat [4][2] 1,2,3,4