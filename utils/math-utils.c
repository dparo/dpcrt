/*
 * Copyright (C) 2018  Davide Paro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "math-utils.h"




float32
gauss_erf_approx_f32(float32 x)
{
    // https://en.wikipedia.org/wiki/Error_function#Approximation_with_elementary_functions

    const float32 denominator = (1.0f
                                 + 0.278393f * x
                                 + 0.230389f * powf(x, 2)
                                 + 0.000972f * powf(x, 3)
                                 + 0.078108f * powf(x, 4));
    const float32 result = 1.0f - (1.0f / denominator);
    return result;
}

float64
gauss_erf_approx_f64(float64 x)
{
    // https://en.wikipedia.org/wiki/Error_function#Approximation_with_elementary_functions

    const float64 denominator = (1.0
                                 + 0.278393 * x
                                 + 0.230389 * pow(x, 2)
                                 + 0.000972 * pow(x, 3)
                                 + 0.078108 * pow(x, 4));
    const float64 result = 1.0 - (1.0 / denominator);
    return result;
}



float32
gaussian_pdf_f32(float32 x,
                 float32 avg,     // mu: expected value = E[x]
                 float32 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const float32 square_root_of_2_times_pi = (float32) 2.50662827463f;
    const float32 sigma_squared = powf(sigma, 2);
    const float32 exponent_term = (- powf(x - avg, 2)) / (2 * sigma_squared);
    const float32 exponential_term = expf(exponent_term);
    const float32 result = (1.0f / (sigma * square_root_of_2_times_pi) ) * exponential_term;
    return result;
}


float64
gaussian_pdf_f64(float64 x,
                 float64 avg,     // mu: expected value = E[x]
                 float64 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const float64 square_root_of_2_times_pi = (float64) 2.50662827463;
    const float64 sigma_squared = pow(sigma, 2);
    const float64 exponent_term = (- pow(x - avg, 2)) / (2 * sigma_squared);
    const float64 exponential_term = exp(exponent_term);
    const float64 result = (1.0f / (sigma * square_root_of_2_times_pi) ) * exponential_term;
    return result;
}


float32
gaussian_cdf_approx_f32(float32 x,
                        float32 avg,     // mu: expected value = E[x]
                        float32 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const float32 square_root_of_2 = 1.41421356237f;
    const float32 erf_term = ((x - avg) / (sigma * square_root_of_2));
    const float32 result = 0.5f * (1.0f + gauss_erf_approx_f32( erf_term ));
    return result;
}


float64
gaussian_cdf_approx_f64(float64 x,
                        float64 avg,     // mu: expected value = E[x]
                        float64 sigma)   // std_dev = sigma ;; Var(x) = pow(sigma, 2)
{
    const float64 square_root_of_2 = 1.41421356237;
    const float64 erf_term = ((x - avg) / (sigma * square_root_of_2));
    const float64 result = 0.5 * (1.0 + gauss_erf_approx_f64( erf_term ));
    return result;
}



float32
laplace_pdf_f32(float32 x,
                float32 mu,  // mu = location parameter
                float32 b)   // b = diversity: eg a scaling factor
{
    const float32 sign_of_x_minus_mu = (x - mu < 0) ? - (x - mu) : x - mu;
    const float32 power = (-(sign_of_x_minus_mu)) / b;
    const float32 exponential_term = expf(power);
    const float32 result = (1.0f / ( 2 * b) ) * exponential_term;
    return result;
}



float64
laplace_pdf_f64(float64 x,
                float64 mu,  // mu = location parameter
                float64 b)   // b = diversity: eg a scaling factor
{
    const float64 sign_of_x_minus_mu = (x - mu < 0) ? - (x - mu) : x - mu;
    const float64 exponent_term = (-(sign_of_x_minus_mu)) / b;
    const float64 exponential_term = exp(exponent_term);
    const float64 result = (1.0f / ( 2 * b) ) * exponential_term;
    return result;
}


float32
laplace_cdf_f32(float32 x,
                float32 mu,  // mu = location parameter
                float32 b)   // b = diversity: eg a scaling factor
{
    float32 result;
    if (x <= mu) {
        const float32 exponent_term = (x - mu) / b;
        result = 0.5f * expf(exponent_term);
    } else {
        const float32 exponent_term = -((x - mu) / b);
        result = 1.0f - 0.5f * expf(exponent_term);
    }
    return result;
}


float64
laplace_cdf_f64(float64 x,
                float64 mu,  // mu = location parameter
                float64 b)   // b = diversity: eg a scaling factor
{
    float64 result;
    if (x <= mu) {
        const float64 exponent_term = (x - mu) / b;
        result = 0.5 * exp(exponent_term);
    } else {
        const float64 exponent_term = -((x - mu) / b);
        result = 1.0 - 0.5 * exp(exponent_term);
    }
    return result;
}



float64
stirling_binomial(uint32 n)
{
    float64 result;
    if (n < 8)
    {
        static const uint32 table[8] = {
            1, 1, 2, 6, 24, 120, 720, 5040
        };
        result = (float64) table[n];
    }
    else
    {
        // Otherwise compute the stirling approximation:
        //     https://en.wikipedia.org/wiki/Stirling%27s_approximation
        const float64 e = EULER_NUM;
        const float64 div_term = (float64) n / e;
        const float64 power_term = powl(div_term, n);
        result = sqrt( 2 * PI * n) * power_term;
    }
    return result;
}
