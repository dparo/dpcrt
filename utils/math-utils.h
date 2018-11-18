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

#ifndef MATH_H
#define MATH_H

#include "utils.h"
#include "types.h"
#include <math.h>


__BEGIN_DECLS

#ifndef PI
# define PI (3.14159265359)
#endif

#define EULER_NUM 2.71828182846


// @NOTE ::
// - https://en.wikipedia.org/wiki/Probability_density_function
//       `pdf` shorthand stands for **Probability Density Function**
// - https://en.wikipedia.org/wiki/Cumulative_distribution_function
//       `cdf` shorthand stands for **Cumulative Distribution Function**



// Approximated computation of the `gauss error function` defined
//  as: (1 / sqrt(PI)) * INTEGRAL[-x, x] { exp( -(t^2) ) }
float32
gauss_erf_approx_f32(float32 x);

float64
gauss_erf_approx_f64(float64 x);


float32
gaussian_pdf_f32(float32 x,
                 float32 avg,     // mu: expected value = E[x]
                 float32 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)

float64
gaussian_pdf_f64(float64 x,
                 float64 avg,     // mu: expected value = E[x]
                 float64 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)




float32
gaussian_cdf_approx_f32(float32 x,
                        float32 avg,     // mu: expected value = E[x]
                        float32 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)

float64
gaussian_cdf_approx_f64(float64 x,
                        float64 avg,     // mu: expected value = E[x]
                        float64 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)


float32
laplace_pdf_f32(float32 x,
                float32 mu,  // mu = location parameter
                float32 b);  // b = diversity: eg a scaling factor

float64
laplace_pdf_f64(float64 x,
                float64 mu,  // mu = location parameter
                float64 b);  // b = diversity: eg a scaling factor


float32
laplace_cdf_f32(float32 x,
                float32 mu,  // mu = location parameter
                float32 b);  // b = diversity: eg a scaling factor


float64
laplace_cdf_f64(float64 x,
                float64 mu,  // mu = location parameter
                float64 b);  // b = diversity: eg a scaling factor


float64
stirling_binomial(uint32 n);

__END_DECLS

#endif
