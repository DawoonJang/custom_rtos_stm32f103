// #define DEBUG

/* SYSTEM */
#define SYSCLK 72000000 // 72MHz
#define HCLK SYSCLK
#define PCLK2 HCLK
#define PCLK1 (HCLK / 2)
#define TIMXCLK ((HCLK == PCLK1) ? (PCLK1) : (PCLK1 * 2))

#define RAM_START 0x20000000
#define RAM_END 0x20004FFF
#define HEAP_BASE (((unsigned int)&__ZI_LIMIT__ + 0x7) & ~0x7)
#define HEAP_SIZE (2 * 1024)
#define HEAP_LIMIT (HEAP_BASE + HEAP_SIZE)

/* OS */
#define MAX_TCB (5)

#define PRIO_HIGHEST (0)
#define PRIO_LOWEST (10)
#define NUM_PRIO (PRIO_LOWEST - PRIO_HIGHEST + 1)

#define STACK_SIZE (4 * 1024)
#define INIT_PSR (0x01000000)
#define TICK_MS (5)

#define MAX_QUEUE (5)
#define QUEUE_ARR_SIZE (1024)