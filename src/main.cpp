#include "include/device_driver.h"
#include "include/os.h"
#include "include/sqe.h"
#include "include/task.h"

#include "arm_math.h"

#define FFT_LENGTH 16 // Set the FFT length (must be a power of 2)

int main(void)
{

    // Define the input and output buffers
    q15_t pSrc[FFT_LENGTH] = {// Example input (real data)
                              1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    developmentVerify();

    // rtos.createTask(ReceiveAndPlayTask, nullptr, 1, 1024);
    rtos.scheduleTask();

    while (1)
    {
        ;
    }

    return 0;
}

// #define FFT_LENGTH 16 // Set the FFT length (must be a power of 2)

// // Define the input and output buffers
// q15_t pSrc[FFT_LENGTH] = { // Example input (real data)
//     1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

// q15_t pDst[2 * FFT_LENGTH]; // Output buffer (size 2 * FFT_LENGTH)

// arm_rfft_instance_q15 S; // Instance of the RFFT structure

// // Define twiddle factor tables (for simplicity, use dummy values here)
// q15_t pTwiddleAReal[FFT_LENGTH]; // Real part of twiddle factors
// q15_t pTwiddleBReal[FFT_LENGTH]; // Imaginary part of twiddle factors

// // Define the coefficients for the twiddle factors (dummy initialization)
// void initTwiddleTables()
// {
//     for (int i = 0; i < FFT_LENGTH; i++)
//     {
//         pTwiddleAReal[i] = (q15_t)(i + 1); // Simple linear coefficients for demonstration
//         pTwiddleBReal[i] = (q15_t)(i + 1); // Same as above
//     }
// }

// void printBuffer(q15_t *buffer, uint32_t length)
// {
//     for (uint32_t i = 0; i < length; i++)
//     {
//         Uart_Printf("%d ", buffer[i]);
//     }
//     Uart_Printf("\n");
// }

// int main()
// {
//     // Initialize the RFFT/RIFFT instance
//     S.fftLenReal = FFT_LENGTH;
//     S.pTwiddleAReal = pTwiddleAReal;
//     S.pTwiddleBReal = pTwiddleBReal;
//     S.twidCoefRModifier = 1; // A modifier for the twiddle coefficients (depends on the FFT size)
//     S.ifftFlagR = 0;         // Set to 0 for RFFT, 1 for RIFFT
//     S.bitReverseFlagR = 1;   // Bit reversal flag (usually set to 1 for most FFTs)

//     // Initialize twiddle tables
//     initTwiddleTables();

//     // Print the original input
//     Uart_Printf("Input buffer:\n");
//     printBuffer(pSrc, FFT_LENGTH);

//     // Perform the Real FFT (RFFT)
//     arm_rfft_q15(&S, pSrc, pDst);

//     // Print the RFFT output
//     Uart_Printf("RFFT output (Real part):\n");
//     printBuffer(pDst, 2 * FFT_LENGTH);

//     // Set up for the Inverse FFT (RIFFT) - using the same instance and buffers
//     S.ifftFlagR = 1; // Set to 1 for RIFFT

//     // Perform the Inverse Real FFT (RIFFT)
//     arm_rfft_q15(&S, pDst, pSrc);

//     // Print the RIFFT output (should resemble the original input)
//     Uart_Printf("RIFFT output (Real part):\n");
//     printBuffer(pSrc, FFT_LENGTH);

//     return 0;
// }