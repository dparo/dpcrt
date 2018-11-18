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

#ifndef STATISTICS
#define STATISTICS


#include "utils.h"
#include "types.h"
#include "dpcc_math.h"

__BEGIN_DECLS


// @NOTE ::
// - https://en.wikipedia.org/wiki/Probability_density_function
//       `pdf` shorthand stands for **Probability Density Function**
// - https://en.wikipedia.org/wiki/Cumulative_distribution_function
//       `cdf` shorthand stands for **Cumulative Distribution Function**



// Approximated computation of the `gauss error function` defined
//  as: (1 / sqrt(PI)) * INTEGRAL[-x, x] { exp( -(t^2) ) }
F32
gauss_erf_approx_f32(F32 x);

F64
gauss_erf_approx_f64(F64 x);


F32
gaussian_pdf_f32(F32 x,
                 F32 avg,     // mu: expected value = E[x]
                 F32 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)

F64
gaussian_pdf_f64(F64 x,
                 F64 avg,     // mu: expected value = E[x]
                 F64 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)




F32
gaussian_cdf_approx_f32(F32 x,
                        F32 avg,     // mu: expected value = E[x]
                        F32 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)

F64
gaussian_cdf_approx_f64(F64 x,
                        F64 avg,     // mu: expected value = E[x]
                        F64 sigma);  // std_dev = sigma ;; Var(x) = pow(sigma, 2)


F32
laplace_pdf_f32(F32 x,
                F32 mu,  // mu = location parameter
                F32 b);  // b = diversity: eg a scaling factor

F64
laplace_pdf_f64(F64 x,
                F64 mu,  // mu = location parameter
                F64 b);  // b = diversity: eg a scaling factor


F32
laplace_cdf_f32(F32 x,
                F32 mu,  // mu = location parameter
                F32 b);  // b = diversity: eg a scaling factor


F64
laplace_cdf_f64(F64 x,
                F64 mu,  // mu = location parameter
                F64 b);  // b = diversity: eg a scaling factor


F64
stirling_binomial(uint32 n);

__END_DECLS

#endif
