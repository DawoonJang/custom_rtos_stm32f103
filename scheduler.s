	.syntax unified
	.thumb

	.text

	.align 4

    .extern currentTaskGlobal
    .extern SwitchingTask

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

    bl      SwitchingTask

    cpsie   i
    pop     {r3, lr}

    ldr     r2, [r3] 
    ldr     r0, [r2]

    ldmia   r0!, {r4-r11}\

    msr     psp, r0
    bx      lr

    .global _SVC_Handler
	.type 	_SVC_Handler, %function
_SVC_Handler:
	ldr		R3, =currentTaskGlobal
	ldr		R1, [R3]
	ldr		R0, [R1]
	ldmia	R0!, {R4-R11}
	msr		PSP, R0

	orr		LR, LR, #0xd
	bx		LR

	.global _OS_Start_First_Task
	.type 	_OS_Start_First_Task, %function
_OS_Start_First_Task:
	svc #0
