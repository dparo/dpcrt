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
#ifndef MATH_H
#define MATH_H

#include "utils.h"
#include "types.h"
#include <math.h>


__BEGIN_DECLS

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


__END_DECLS

#endif
