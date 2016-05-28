mov r2, #1          ;r2 = 1
lsl r2, #18         ;r2 = 0x40000
ldr r0, =0x20200004 ;loads r0 with the address 0x20200004
str r2, [r0]        ;store a 001 in the 3-bit gorup for Pins 10-19
                    ;Pin 16 -> out
		    ;Before setting Pin 16 ON, we have to set it OFF
mov r1, #1
mov r3, #1
lsl r3, #16         ;r3 = 0x10000
str r3, [r0,#36]    ;pin off initially

loop:
str r3, [r0,#36]    ;before each iteration bit is set off

mov r4, #8388608    ;r4 init with large enough number, r4 = #80 00 00
wait:               ;break between flashes
sub r4,r4,#1 
cmp r4,#0
bne wait

str r3,[r0,#24]     ;pin on

mov r4, #8388608    ;r4 init with large enough number, r4 = #80 00 00
wait:               ;break between flashes
sub r4,r4,#1 
cmp r4,#0
bne wait

cmp r1, #0          ;r1 always holds #1, which is not equal to #0
bne loop            ;next loop iteration 
andeq r0,r0,r0      ;halt execution(never reached)



