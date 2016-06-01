mov r2, #1
lsl r2, #18
ldr r0, =0x20200004
ldr r3, [r0]
orr r3, r3, r2
str r3, [r0]

mov r3, #1
lsl r3, #16
loop:
str r3, [r0,#36]

mov r4, #4128768
waitdelay:
sub r4, r4, #1
cmp r4, #0
bne waitdelay

str r3, [r0, #24]

mov r4, #4128768
wait:
sub r4, r4, #1
cmp r4, #0
bne wait

b loop
andeq r0, r0, r0
