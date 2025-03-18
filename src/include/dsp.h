#pragma once
#include "arm_math.h"
#include "device_driver.h"

#define FIR_ORDER 32
#define IIR_ORDER 4

#define FFT_LENGTH 128
#define FFT_HALF_LENGTH (FFT_LENGTH / 2)
#define SAMPLE_RATE 512
#define SIGNAL_FREQ 16

#define LOG2N (log2(FFT_LENGTH))

#ifndef PI
#define PI (3.14159265358979f)
#endif

enum class FilterOption
{
    Normal,
    LPF,
    HPF,
    BPF
};

class DSP
{
  private:
    float WR[FFT_HALF_LENGTH];
    float WI[FFT_HALF_LENGTH];

    void precomputeTwiddleFactors(const unsigned short N);

  public:
    DSP();

    FilterOption filterOption;

    int FFT(const float *pSrc, float *const pDstReal, float *const pDstImag, const size_t length);
    void FIR_Filter(const float *const input, float *const output, const size_t length,
                    const float *const coefficients);

    void IIR_Filter(const float *const input, float *const output, const size_t length,
                    const float *const b_coefficients, const float *const a_coefficients);

    void changeFilterOption(void);

    const float FIR_LPF_Coefficients_20[FIR_ORDER] = {
        0.0022,  0.0042,  0.0061,  0.0076,  0.0087,  0.0091,  0.0087,  0.0076,  0.0061,  0.0042,  0.0022,
        0.0001,  -0.0019, -0.0037, -0.0053, -0.0067, -0.0076, -0.0081, -0.0081, -0.0076, -0.0067, -0.0053,
        -0.0037, -0.0019, 0.0001,  0.0022,  0.0042,  0.0061,  0.0076,  0.0087,  0.0091,  0.0087};

    const float FIR_HPF_Coefficients_30[FIR_ORDER] = {
        0.0006,  0.0013,  0.0024,  0.0038,  0.0054,  0.0065, 0.0060,  0.0027,  -0.0067, -0.0042, -0.0153,
        -0.0305, -0.0488, -0.0685, -0.0874, -0.1031, 0.8825, -0.1135, -0.1031, -0.0874, -0.0685, -0.0488,
        -0.0305, -0.0153, -0.0042, 0.0027,  0.0060,  0.0065, 0.0054,  0.0038,  0.0024,  0.0013};

    const float FIR_HPF_Coefficients_100[FIR_ORDER] = {
        0.0016, 0.0002,  -0.0026, -0.0011, 0.0053, 0.0039,  -0.0096, -0.0101, // 1 ~ 8
        0.0152, 0.0221,  -0.0211, -0.0446, 0.0263, 0.0935,  -0.0299, -0.3135, // 9 ~ 16
        0.5304, -0.3135, -0.0299, 0.0935,  0.0263, -0.0446, -0.0211, 0.0221,  // 17 ~ 24
        0.0152, -0.0101, -0.0096, 0.0039,  0.0053, -0.0011, -0.0026, 0.0002   // 25 ~ 32
    };

    const float FIR_HPF_Coefficients_200[FIR_ORDER] = {
        -0.0016, 0.0015,  -0.0005, -0.0018, 0.0053, -0.0079, 0.0064,  0.0016,  // 1 ~ 8
        -0.0152, 0.0285,  -0.0316, 0.0147,  0.0264, -0.0865, 0.1509,  -0.2005, // 9 ~ 16
        0.2191,  -0.2005, 0.1509,  -0.0865, 0.0264, 0.0147,  -0.0316, 0.0285,  // 17 ~ 24
        -0.0152, 0.0016,  0.0064,  -0.0079, 0.0053, -0.0018, -0.0005, 0.0015,  // 25 ~ 32
    };

    const float FIR_LPF_Coefficients_200[FIR_ORDER] = {
        0.0016, -0.0015, 0.0005,  0.0018,  -0.0053, 0.0079,  -0.0064, -0.0016, // 1 ~ 8
        0.0152, -0.0285, 0.0315,  -0.0147, -0.0263, 0.0862,  -0.1504, 0.1998,  // 9 ~ 16
        0.7800, 0.1998,  -0.1504, 0.0862,  -0.0263, -0.0147, 0.0315,  -0.0285, // 17 ~ 24
        0.0152, -0.0016, -0.0064, 0.0079,  -0.0053, 0.0018,  0.0005,  -0.0015  // 25 ~ 32
    };

    const float IIR_LPF_B_Coef_20[IIR_ORDER + 1] = {0.0002f, 0.0007f, 0.0010f, 0.0007f, 0.0002f};
    const float IIR_LPF_A_Coef_20[IIR_ORDER + 1] = {1.0000f, -3.3594f, 4.2755f, -2.4390f, 0.5256f};

    const float IIR_LPF_B_Coef_200[IIR_ORDER + 1] = {0.3987f, 1.5948, 2.3922f, 1.5948f, 0.3987f};
    const float IIR_LPF_A_Coef_200[IIR_ORDER + 1] = {1.0000f, 2.2187f, 2.0828f, 0.9185f, 0.1590f};

    const float IIR_HPF_B_Coef_20[IIR_ORDER + 1] = {0.7250f, -2.8999f, 4.3498f, -2.8999f, 0.7250f};
    const float IIR_HPF_A_Coef_20[IIR_ORDER + 1] = {1.0000f, -3.3594f, 4.2755f, -2.4390f, 0.5256f};

    const float IIR_HPF_B_Coef_120[IIR_ORDER + 1] = {0.1137f, -0.4549f, 0.6823f, -0.4549f, 0.1137f};
    const float IIR_HPF_A_Coef_120[IIR_ORDER + 1] = {1.0000f, -0.2441f, 0.5049f, -0.0518f, 0.0188f};

    const float IIR_HPF_B_Coef_200[IIR_ORDER + 1] = {0.0065f, -0.0261f, 0.0392f, -0.0261f, 0.0065f};
    const float IIR_HPF_A_Coef_200[IIR_ORDER + 1] = {1.0000f, 2.2187f, 2.0828f, 0.9185f, 0.1590f};
};
