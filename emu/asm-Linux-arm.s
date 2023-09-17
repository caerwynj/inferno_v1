
	.file	"arm-tas-v5.S"
/*
 * ulong _tas(ulong*);
 */
	.align	2
	.global	_tas
	.type	_tas, %function
_tas:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	@ lr needed for prologue
	mov	r3, #1
	mov	r1, r0
	swp	r0, r3, [r1]
	bx	lr
	.size	_tas, .-_tas


/*
 * ulong umult(ulong m1, ulong m2, ulong *hi)
 */

        .align  2
        .global umult
        .type   umult, %function
umult:
        @ args = 0, pretend = 0, frame = 12
        @ frame_needed = 1, uses_anonymous_args = 0
        mov     ip, sp
        stmfd   sp!, {fp, ip, lr, pc}
        sub     fp, ip, #4
        sub     sp, sp, #12
        str     r0, [fp, #-16]
        str     r1, [fp, #-20]
        str     r2, [fp, #-24]
        ldr     r1, [fp, #-16]
        ldr     r2, [fp, #-20]
        umull   r0, r3, r1, r2
        ldr     r1, [fp, #-24]
        str     r3, [r1]
        ldmea   fp, {fp, sp, pc}
        .size   umult, .-umult
