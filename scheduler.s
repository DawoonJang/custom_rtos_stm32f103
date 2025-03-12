	.syntax unified
	.thumb

	.text

	.align 4

    .extern currentTaskGlobal
    .extern switchingTask

	.global _PendSV_Handler
	.type 	_PendSV_Handler, %function
_PendSV_Handler:
    mrs     r0, psp

    stmdb   r0!, {r4-r11}

    ldr     r3, =currentTaskGlobal
    ldr     r2, [r3]
    str     r0, [r2]

    push    {r3, lr}
    cpsid   i

    bl      switchingTask

    cpsie   i
    pop     {r3, lr}

    ldr     r2, [r3] 
    ldr     r0, [r2]

    ldmia   r0!, {r4-r11}

    msr     psp, r0
    bx      lr

    .global _SVC_Handler
	.type 	_SVC_Handler, %function
_SVC_Handler:
    ldr     r3, =currentTaskGlobal
    ldr     r1, [r3]
    ldr     r0, [r1]
    ldmia   r0!, {r4-r11}
    msr     psp, r0

    orr     lr, lr, #0xd
    bx      lr