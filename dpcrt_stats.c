#include "dpcrt_stats.h"

void cdf_print(double *cdf, int32_t N)
{
    int32_t i = 0;

    printf("\ncdf[0..%d] = [\n", N);
    for (; i < N && i < 32; i++) {
        printf("    %f, ", cdf[i]);
        if (((i + 1) % 8) == 0) {
            printf("\n");
        }
    }

    if ( i < N ) {
        printf("\n   ...\n\n");

        for (i = MAX(i, N - 32); i < N; i++) {
            printf("    %f, ", cdf[i]);
            if ((i + 1) % 8 == 0) {
                printf("\n");
            }
        }
    }

    printf("\n];\n\n");
}

void cdf_normalize(double *cdf, int32_t N)
{
    int32_t f = 0.0;
    double div = cdf[N - 1];
    for (int32_t i = 0; i < N; i++) {
        assert(cdf[i] >= f);
        f = cdf[i];
        cdf[i] = cdf[i] / div;
    }

    assert(cdf[N - 1] <= 1.0);
}


void init_uniform_cdf(double *cdf, int32_t N)
{
    double sum = 0.0;

    for (int32_t i = 0; i < N; i++) {
        double t = (double) i / (double) N;
        sum += t;
        cdf[i] = sum;
    }
}


int32_t icdf(double *cdf, int32_t N, double p)
{
    assert(cdf[N - 1] <= 1.0);

    assert(p >= 0 && p <= cdf[N - 1]);

    double first_cdf = cdf[0];
    double last_cdf  = cdf[N - 1];

    if (p >= last_cdf) {
        assert(!"Invalid code path\n");
    } else if (p <= 0) {
        if (p < 0.0) {
            assert(!"Invalid code path\n");
        }
    }

    if (p <= first_cdf) {
        return 0;
    }

    int32_t start = 1;
    int32_t end = N;

    // Binary search the value in the CDF
    while (start != end) {
        int32_t m = start + (end - start) / 2;

        if (p > cdf[m - 1] && p <= cdf[m]) {
            return m;
        } else if (p > cdf[m]) {
            start = m + 1;
        } else {
            end = m;
        }
    }

    assert(!"Invalid code path. We should always find a possible index, otherwise this is an internal bug inside this function\n");
    return INT32_MIN;
}


int32_t icdf_sample_once(double *cdf, int32_t N)
{
    double p = (double) rand() / (double) RAND_MAX;
    assert(p >= 0.0 && p <= 1.0);
    return icdf(cdf, N, p);
}

void icdf_sample_n(double *cdf, int32_t N, int32_t *samples, int32_t nsamples)
{
    for (int32_t i = 0; i < nsamples; i++) {
        double p = (double) rand() / (double) RAND_MAX;
        assert(p >= 0.0 && p <= 1.0);
        samples[i] = icdf(cdf, N, p);
    }
}


void icdf_sample_without_replacement(double *cdf, int32_t N, int32_t *samples, int32_t nsamples)
{
    assert(nsamples);

    double *cp = malloc(N * sizeof(cp[0]));
    memcpy(cp, cdf, N * sizeof(cp[0]));

    for (int32_t i = 0; i < nsamples; i++) {
        int32_t s = icdf_sample_once(cdf, N);
        samples[i] = s;
        cp[s] = s > 0 ? cp[s - 1] : cp[s] = 0.0;        // Reset the CDF so we will not be able to sample it again
    }

    free(cp);
}
