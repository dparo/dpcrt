/*
 * Copyright (C) 2018  Davide Paro
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "dpcrt_math.h"
#include <math.h>

F32
gauss_erf_approx_f32(F32 x)
{
    // https://en.wikipedia.org/wiki/Error_function#Approximation_with_elementary_functions

    const F32 denominator = (1.0f
                             + 0.278393f * x
                             + 0.230389f * powf(x, 2)
                             + 0.000972f * powf(x, 3)
                             + 0.078108f * powf(x, 4));
    const F32 result = 1.0f - (1.0f / denominator);
    return result;
}

F64
gauss_erf_approx_f64(F64 x)
{
    // https://en.wikipedia.org/wiki/Error_function#Approximation_with_elementary_functions

    const F64 denominator = (1.0
                             + 0.278393 * x
                             + 0.230389 * pow(x, 2)
                             + 0.000972 * pow(x, 3)
                             + 0.078108 * pow(x, 4));
    const F64 result = 1.0 - (1.0 / denominator);
    return result;
}



F32
gaussian_pdf_f32(F32 x,
                 F32 avg,     // mu: expected value = E[x]
                 F32 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const F32 square_root_of_2_times_pi = (F32) 2.50662827463f;
    const F32 sigma_squared = powf(sigma, 2);
    const F32 exponent_term = (- powf(x - avg, 2)) / (2 * sigma_squared);
    const F32 exponential_term = expf(exponent_term);
    const F32 result = (1.0f / (sigma * square_root_of_2_times_pi) ) * exponential_term;
    return result;
}


F64
gaussian_pdf_f64(F64 x,
                 F64 avg,     // mu: expected value = E[x]
                 F64 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const F64 square_root_of_2_times_pi = (F64) 2.50662827463;
    const F64 sigma_squared = pow(sigma, 2);
    const F64 exponent_term = (- pow(x - avg, 2)) / (2 * sigma_squared);
    const F64 exponential_term = exp(exponent_term);
    const F64 result = (1.0 / (sigma * square_root_of_2_times_pi) ) * exponential_term;
    return result;
}


F32
gaussian_cdf_approx_f32(F32 x,
                        F32 avg,     // mu: expected value = E[x]
                        F32 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const F32 square_root_of_2 = 1.41421356237f;
    const F32 erf_term = ((x - avg) / (sigma * square_root_of_2));
    const F32 result = 0.5f * (1.0f + gauss_erf_approx_f32( erf_term ));
    return result;
}


F64
gaussian_cdf_approx_f64(F64 x,
                        F64 avg,     // mu: expected value = E[x]
                        F64 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const F64 square_root_of_2 = 1.41421356237;
    const F64 erf_term = ((x - avg) / (sigma * square_root_of_2));
    const F64 result = 0.5 * (1.0 + gauss_erf_approx_f64( erf_term ));
    return result;
}



F32
laplace_pdf_f32(F32 x,
                F32 mu,  // mu = location parameter
                F32 b)   // b = diversity: eg a scaling factor
{
    const F32 sign_of_x_minus_mu = (x - mu < 0) ? - (x - mu) : x - mu;
    const F32 power = (-(sign_of_x_minus_mu)) / b;
    const F32 exponential_term = expf(power);
    const F32 result = (1.0f / ( 2 * b) ) * exponential_term;
    return result;
}



F64
laplace_pdf_f64(F64 x,
                F64 mu,  // mu = location parameter
                F64 b)   // b = diversity: eg a scaling factor
{
    const F64 sign_of_x_minus_mu = (x - mu < 0) ? - (x - mu) : x - mu;
    const F64 exponent_term = (-(sign_of_x_minus_mu)) / b;
    const F64 exponential_term = exp(exponent_term);
    const F64 result = (1.0 / ( 2 * b) ) * exponential_term;
    return result;
}


F32
laplace_cdf_f32(F32 x,
                F32 mu,  // mu = location parameter
                F32 b)   // b = diversity: eg a scaling factor
{
    F32 result;
    if (x <= mu) {
        const F32 exponent_term = (x - mu) / b;
        result = 0.5f * expf(exponent_term);
    } else {
        const F32 exponent_term = -((x - mu) / b);
        result = 1.0f - 0.5f * expf(exponent_term);
    }
    return result;
}


F64
laplace_cdf_f64(F64 x,
                F64 mu,  // mu = location parameter
                F64 b)   // b = diversity: eg a scaling factor
{
    F64 result;
    if (x <= mu) {
        const F64 exponent_term = (x - mu) / b;
        result = 0.5 * exp(exponent_term);
    } else {
        const F64 exponent_term = -((x - mu) / b);
        result = 1.0 - 0.5 * exp(exponent_term);
    }
    return result;
}



F64
stirling_binomial(uint32 n)
{
    F64 result;
    if (n < 8)
    {
        static const uint32 table[8] = {
            1, 1, 2, 6, 24, 120, 720, 5040
        };
        result = (F64) table[n];
    }
    else
    {
        // Otherwise compute the stirling approximation:
        //     https://en.wikipedia.org/wiki/Stirling%27s_approximation
        const F64 e = EULER_NUM;
        const F64 div_term = (F64) n / e;
        const F64 power_term = pow(div_term, n);
        result = sqrt( 2 * PI * n) * power_term;
    }
    return result;
}
