#include "dsp.h"
#include <functional>
DSP dsp;

DSP::DSP()
{
    precomputeTwiddleFactors(FFT_LENGTH);
}

void DSP::precomputeTwiddleFactors(const unsigned short N)
{
    unsigned short N2 = N >> 1;
    float T = 2 * PI / N;

    for (long i = 0; i < N2; ++i)
    {
        WR[i] = cos(T * i);
        WI[i] = -sin(T * i);
    }
}

int DSP::FFT(const float *pSrc, float *const pDstReal, float *const pDstImag, const size_t length)
{
    /* 1. check 2^n count -> if else then return -1 */
    if ((length != 0) && ((length & (length - 1)) != 0))
        return -1;

    /* 2. initialize destination arrays */
    for (size_t i = 0; i < length; ++i)
    {
        pDstReal[i] = pSrc[i];
        pDstImag[i] = 0.0;
    }

    /* 4. shuffle input array index to calculate bottom-up style FFT */
    size_t log2N = (size_t)log2(length);
    double tmp;
    for (size_t n = 1; n < length - 1; ++n)
    {
        size_t m = 0; /* reversed num of n*/
        for (size_t i = 0; i < log2N; ++i)
        {
            m |= ((n >> i) & 1) << (log2N - i - 1); /* reverse the bits */
        }
        if (n < m)
        { /* exchange */
            tmp = pDstReal[n];
            pDstReal[n] = pDstReal[m];
            pDstReal[m] = tmp;
        }
    }

    /* 5. execute fft */
    for (size_t loop = 0; loop < log2N; ++loop)
    {
        size_t regionSize = 1 << (loop + 1);    /* if N=8: 2 -> 4 -> 8 */
        size_t kJump = 1 << (log2N - loop - 1); /* if N=8: 4 -> 2 -> 1 */
        size_t half = regionSize >> 1;
        for (size_t i = 0; i < length; i += regionSize)
        {
            size_t blockEnd = i + half - 1;
            size_t k = 0;
            for (size_t j = i; j <= blockEnd; ++j)
            { /* j start from i */
                float TR = WR[k] * pDstReal[j + half] - WI[k] * pDstImag[j + half];
                float TI = WI[k] * pDstReal[j + half] + WR[k] * pDstImag[j + half];

                pDstReal[j + half] = pDstReal[j] - TR;
                pDstImag[j + half] = pDstImag[j] - TI;

                pDstReal[j] = pDstReal[j] + TR;
                pDstImag[j] = pDstImag[j] + TI;

                k += kJump;
            }
        }
    }
    return 0;
}

void DSP::FIR_Filter(const float *const input, float *const output, const size_t length,
                     const float *const coefficients)
{

    std::fill(output, output + length, 0.0f);

    for (size_t i = 0; i < length; i++)
    {
        size_t j = 0;

        for (; j + 3 < FILTER_ORDER; j += 4) // loop unroll every 4 elements
        {
            if (i >= j + 3)
            {
                output[i] += coefficients[j] * input[i - j] + coefficients[j + 1] * input[i - (j + 1)] +
                             coefficients[j + 2] * input[i - (j + 2)] + coefficients[j + 3] * input[i - (j + 3)];
            }
        }

        for (; j < FILTER_ORDER; j++)
        {
            if (i >= j)
            {
                output[i] += coefficients[j] * input[i - j];
            }
        }
    }
}

void DSP::changeFilterOption(void)
{
    switch (filterOption)
    {
    case FilterOption::Normal:
        filterOption = FilterOption::LPF;
        break;
    case FilterOption::LPF:
        filterOption = FilterOption::HPF;
        break;
    case FilterOption::HPF:
        filterOption = FilterOption::BPF;
        break;
    case FilterOption::BPF:
        filterOption = FilterOption::Normal;
        break;
    }
}