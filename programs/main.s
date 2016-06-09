ldr r0, =0x20200000
ldr r1, =0x20200028
ldr r2, =0x2020001C
ldr r3, [r0]
orr r3, r3, #0x1240
str r3, [r0]

mov r4, #4
mov r5, #8
mov r6, #16

loop:

str r4, [r1]
str r5, [r1]
str r6, [r1]

ldr r7, =10000000
wait1:
sub r7, r7, #1
cmp r7, #0
bne wait1

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r6, [r2]

ldr r7, =10000000
wait2:
sub r7, r7, #1
cmp r7, #0
bne wait2

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r5, [r2]

ldr r7, =10000000
wait3:
sub r7, r7, #1
cmp r7, #0
bne wait3

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r5, [r2]
str r6, [r2]

ldr r7, =10000000
wait4:
sub r7, r7, #1
cmp r7, #0
bne wait4

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r4, [r2]

ldr r7, =10000000
wait5:
sub r7, r7, #1
cmp r7, #0
bne wait5

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r4, [r2]
str r6, [r2]

ldr r7, =10000000
wait6:
sub r7, r7, #1
cmp r7, #0
bne wait6

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r4, [r2]
str r5, [r2]

ldr r7, =10000000
wait7:
sub r7, r7, #1
cmp r7, #0
bne wait7

str r4, [r1]
str r5, [r1]
str r6, [r1]

str r4, [r2]
str r5, [r2]
str r6, [r2]

ldr r7, =10000000
wait8:
sub r7, r7, #1
cmp r7, #0
bne wait8

b loop
andeq r0, r0, r0
