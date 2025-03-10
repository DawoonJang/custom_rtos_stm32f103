	.syntax unified
	.thumb

	.text

	.align 4

    .extern currentTask
    .extern SwitchingTask

	.global _PendSV_Handler
	.type 	_PendSV_Handler, %function
_PendSV_Handler:
    mrs     r0, psp

    stmdb   r0!, {r4-r11}

    ldr     r3, =currentTask
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
	LDR		R3, =currentTask
	LDR		R1, [R3]
	LDR		R0, [R1]
	LDMIA	R0!, {R4-R11}
	MSR		PSP, R0

	ORR		LR, LR, #0xd
	BX		LR

	.global _OS_Start_First_Task
	.type 	_OS_Start_First_Task, %function
_OS_Start_First_Task:
	SVC #0
	BX LR