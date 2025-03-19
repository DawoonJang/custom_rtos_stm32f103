#pragma once
#include "arm_math.h"
#include "device_driver.h"

#define FIR_ORDER 32
#define IIR_ORDER 4

#define FFT_LENGTH 128
#define FFT_HALF_LENGTH (FFT_LENGTH / 2)
#define SAMPLE_RATE 4000
#define SIGNAL_FREQ 2000

#define LOG2N (log2(FFT_LENGTH))

#ifndef PI
#define PI (3.14159265358979f)
#endif

#ifdef ARM_MATH
#include "arm_math.h"
#define DEFSINE arm_sin_f32
#define DEFCOS arm_cos_f32
#define DEFSQRT arm_sqrt_q15
#else
#include <math.h>
#define DEFSINE sin
#define DEFCOS cos
#define DEFSQRT sqrt
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

    const float FIR_LPF_Coefficients_575[FIR_ORDER] = {
        0.0015,  0.0016,  0.0002,  -0.0028, -0.0056, -0.0040, 0.0044, 0.0153,  0.0173,  0.0011,  // 1 ~ 10
        -0.0288, -0.0496, -0.0312, 0.0409,  0.1490,  0.2472,  0.2869, 0.2472,  0.1490,  0.0409,  // 11 ~ 20
        -0.0312, -0.0496, -0.0288, 0.0011,  0.0173,  0.0153,  0.0044, -0.0040, -0.0056, -0.0028, // 21 ~ 30
        0.0002,  0.0016                                                                          // 31 ~ 32
    };
    const float FIR_HPF_Coefficients_1200[FIR_ORDER] = {
        0.0015,  0.0000, -0.0025, 0.0023,  0.0033,  -0.0078, -0.0000, 0.0151,  -0.0126, -0.0168, // 1 ~ 10
        0.0361,  0.0000, -0.0654, 0.0575,  0.0902,  -0.2998, 0.3997,  -0.2998, 0.0902,  0.0575,  // 11 ~ 20
        -0.0654, 0.0000, 0.0361,  -0.0168, -0.0126, 0.0151,  -0.0000, -0.0078, 0.0033,  0.0023,  // 21 ~ 30
        -0.0025, 0.0000                                                                          // 31 ~ 32
    };

    const float FIR_HPF_Coefficients_575[FIR_ORDER] = {
        -0.0015, -0.0016, -0.0002, 0.0028,  0.0056,  0.0040,  -0.0044, -0.0153, -0.0174, -0.0011, // 1 ~ 10
        0.0289,  0.0497,  0.0313,  -0.0410, -0.1495, -0.2481, 0.7134,  -0.2481, -0.1495, -0.0410, // 11 ~ 20
        0.0313,  0.0497,  0.0289,  -0.0011, -0.0174, -0.0153, -0.0044, 0.0040,  0.0056,  0.0028,  // 21 ~ 30
        -0.0002, -0.0016                                                                          // 31 ~ 32
    };

    const float FIR_LPF_Coefficients_1200[FIR_ORDER] = {
        -0.0015, 0.0000, 0.0025,  -0.0023, -0.0034, 0.0078,  -0.0000, -0.0152, 0.0127,  0.0169,  // 1 ~ 10
        -0.0362, 0.0000, 0.0656,  -0.0576, -0.0904, 0.3006,  0.6011,  0.3006,  -0.0904, -0.0576, // 11 ~ 20
        0.0656,  0.0000, -0.0362, 0.0169,  0.0127,  -0.0152, -0.0000, 0.0078,  -0.0034, -0.0023, // 21 ~ 30
        0.0025,  0.0000                                                                          // 31 ~ 32
    };

    const float IIR_LPF_B_Coef_575[IIR_ORDER + 1] = {0.0161, 0.0644, 0.0966, 0.0644, 0.0161};   // opt 600
    const float IIR_LPF_A_Coef_575[IIR_ORDER + 1] = {1.0000, -1.6722, 1.3829, -0.5388, 0.0858}; // opt 600

    const float IIR_HPF_B_Coef_1200[IIR_ORDER + 1] = {0.0466, -0.1863, 0.2795, -0.1863, 0.0466}; // opt 1300
    const float IIR_HPF_A_Coef_1200[IIR_ORDER + 1] = {1.0000, 0.7821, 0.6800, 0.1827, 0.0301};   // opt 1300

    const float IIR_HPF_B_Coef_575[IIR_ORDER + 1] = {0.2920, -1.1682, 1.7522, -1.1682, 0.2920};
    const float IIR_HPF_A_Coef_575[IIR_ORDER + 1] = {1.0000, -1.6696, 1.3801, -0.5374, 0.0855};

    const float IIR_LPF_B_Coef_1200[IIR_ORDER + 1] = {0.1672, 0.6687, 1.0031, 0.6687, 0.1672};
    const float IIR_LPF_A_Coef_1200[IIR_ORDER + 1] = {1.0000, 0.7821, 0.6800, 0.1827, 0.0301};
};
