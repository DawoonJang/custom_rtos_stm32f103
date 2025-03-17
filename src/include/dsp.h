#pragma once
#include "device_driver.h"
#include "arm_math.h"

#define FILTER_ORDER 32
#define FFT_LENGTH 128
#define SAMPLE_RATE 512
#define SIGNAL_FREQ 16

#define PI 3.14159265358979f
#define Q15_SCALE (1 << 15) // 2^15
#define Q31_SCALE (1 << 31)
#define FLOAT_TO_Q15(x) ((q15_t)((x) * Q15_SCALE))
#define FLOAT_TO_Q31(x) ((q31_t)((x) * Q31_SCALE))
#define MAKE_FREQ_IDX(i) (q15_t)(((float)(i)) / (float)FFT_LENGTH * Q15_SCALE)

class DSP 
{
private:
    void precomputeTwiddleFactors(long N);

public:
    DSP();
    int FFT(long N, const double *pSrc, double *pDstReal, double *pDstImag);
    void FIR_Filter(double *input, double *output, size_t length, const float *coefficients);
    // Low-pass filter coefficients
    float LPF_Coefficients[FILTER_ORDER] = {0.0022,  0.0042,  0.0061,  0.0076,  0.0087,  0.0091,  0.0087,  0.0076,
                                   0.0061,  0.0042,  0.0022,  0.0001,  -0.0019, -0.0037, -0.0053, -0.0067,
                                   -0.0076, -0.0081, -0.0081, -0.0076, -0.0067, -0.0053, -0.0037, -0.0019,
                                   0.0001,  0.0022,  0.0042,  0.0061,  0.0076,  0.0087,  0.0091,  0.0087};
    
    // High-pass filter coefficients
    float HPF_Coefficients[FILTER_ORDER] = {0.0006,  0.0013,  0.0024,  0.0038,  0.0054,  0.0065,  0.0060,  0.0027,
                                   -0.0067, -0.0042, -0.0153, -0.0305, -0.0488, -0.0685, -0.0874, -0.1031,
                                   0.8825,  -0.1135, -0.1031, -0.0874, -0.0685, -0.0488, -0.0305, -0.0153,
                                   -0.0042, 0.0027,  0.0060,  0.0065,  0.0054,  0.0038,  0.0024,  0.0013};
};
