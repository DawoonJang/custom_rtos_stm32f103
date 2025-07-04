#pragma once

// #define DEBUG_ITR
#define DEBUG_PRINT

// #define TESTCASE1
#define TESTCASE2
// #define TESTCASE3

/* SYSTEM */
#define SYSCLK 72000000 // 72MHz
#define HCLK SYSCLK
#define PCLK2 HCLK
#define PCLK1 (HCLK / 2)
#define TIMXCLK ((HCLK == PCLK1) ? (PCLK1) : (PCLK1 * 2))

/* OS */
#define MAX_TCB (5)

#define PRIO_HIGHEST (0)
#define PRIO_LOWEST (10)
#define NUM_PRIO (PRIO_LOWEST - PRIO_HIGHEST + 1)

#define STACK_SIZE (8 * 1024)
#define INIT_PSR (0x01000000)
#define TICK_MS (5)

#define MAX_QUEUE (5)
#define QUEUE_STACK_SIZE (4 * 1024)

#define MAX_MUTEX (20)

/* BUZZER */
#define TIM3_FREQ (8000000)             // Hz
#define TIM3_TICK (1000000 / TIM3_FREQ) // usec
#define TIME3_PLS_OF_1ms (1000 / TIM3_TICK)

/* Player */
#define WAV_HEADER_SIZE 44

#define ARM_MATH
#define WITH_SIGNAL