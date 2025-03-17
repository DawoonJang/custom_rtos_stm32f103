#include "dsp.h"

DSP dsp;

static double WR[FFT_LENGTH >> 1];
static double WI[FFT_LENGTH >> 1];

DSP::DSP()
{
    precomputeTwiddleFactors(FFT_LENGTH);
}

void DSP::precomputeTwiddleFactors(long N)
{
    long N2 = N >> 1;
    double T = 2 * PI / N;

    for (long i = 0; i < N2; ++i)
    {
        WR[i] = cos(T * i);
        WI[i] = -sin(T * i);
    }
}

int DSP::FFT(long N, const double *pSrc, double *pDstReal, double *pDstImag)
{
    /* 1. check 2^n count -> if else then return -1 */
    if ((N != 0) && ((N & (N - 1)) != 0))
        return -1;

    /* 2. initialize destination arrays */
    for (long i = 0; i < N; ++i)
    {
        pDstReal[i] = pSrc[i];
        pDstImag[i] = 0.0;
    }

    /* 4. shuffle input array index to calculate bottom-up style FFT */
    int log2N = (int)log2(N);
    double tmp;
    for (long n = 1; n < N - 1; ++n)
    {
        long m = 0; /* reversed num of n*/
        for (int i = 0; i < log2N; ++i)
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
    for (int loop = 0; loop < log2N; ++loop)
    {
        long regionSize = 1 << (loop + 1);    /* if N=8: 2 -> 4 -> 8 */
        long kJump = 1 << (log2N - loop - 1); /* if N=8: 4 -> 2 -> 1 */
        long half = regionSize >> 1;
        for (long i = 0; i < N; i += regionSize)
        {
            long blockEnd = i + half - 1;
            long k = 0;
            for (long j = i; j <= blockEnd; ++j)
            { /* j start from i */
                double TR = WR[k] * pDstReal[j + half] - WI[k] * pDstImag[j + half];
                double TI = WI[k] * pDstReal[j + half] + WR[k] * pDstImag[j + half];

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

void DSP::FIR_Filter(double *input, double *output, size_t length, const float *coefficients)
{
    for (size_t i = 0; i < length; i++)
    {
        output[i] = 0.0;
        for (size_t j = 0; j < FILTER_ORDER; j++)
        {
            if (i >= j)
            {
                output[i] += coefficients[j] * input[i - j];
            }
        }
    }
}