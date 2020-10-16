#pragma once

#include "dpcrt_allocators.h"
#include "dpcrt_utils.h"
#include <stddef.h>
#include <stdint.h>


void cdf_print(double *cdf, int32_t N);
void cdf_normalize(double *cdf, int32_t N);

void init_uniform_cdf(double *cdf, int32_t N);


// Given a cDF of PMF computes the reciprocal defined as CDF(1 - PMF) without
// explicitly requiring the src PMF and generating the target PMF. The computation
// is found by anallitical considerations.
void init_cdf_with_reciprocal(double *cdf_dest, double *cdf_src, int32_t N);


int32_t icdf(double *cdf, int32_t N, double p);
int32_t icdf_sample_once(double *cdf, int32_t N);
void    icdf_sample_n(double *cdf, int32_t N, int32_t *samples, int32_t nsamples);
void    icdf_sample_without_replacement(double *cdf, int32_t N, int32_t *samples, int32_t nsamples);
