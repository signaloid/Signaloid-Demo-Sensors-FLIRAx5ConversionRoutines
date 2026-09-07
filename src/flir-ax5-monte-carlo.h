/*
 *	Copyright (c) 2024-2026, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#pragma once

#include <stddef.h>

/**
 *	@brief	Monte Carlo calculation kernel. Runs `numberOfMonteCarloIterations`
 *		independent evaluations of the FLIR Ax5 conversion routine into
 *		`monteCarloOutputSamples`. Each iteration calls
 *		`FLIRAx5SetInputVariablesViaUxHwCall` and `FLIRAx5_calculateOutput`
 *		exactly once, identically to the UxHw-mode kernel: this is not a
 *		UxHw-free path, since the sensor's distributional input
 *		parameters are declared via `UxHwDoubleUniformDist(...)` macros
 *		in `kernel.h` and re-sampling them without UxHw calls would be a
 *		semantics change, not a restructure (see the refactor report).
 *
 *		Purely computational: no dispatch-layer struct parameter and no
 *		config macro reads. The caller (`FLIRAx5CalculateOutputMonteCarlo`
 *		in `kernel.c`) sizes `inputVariables`.
 *
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *	@param	countValueToOverrideDefaultDistribution	: Scalar
 *			override for the sensor counts input, or `NaN` to use
 *			the per-iteration sampled value in `inputVariables` instead.
 *	@param	inputVariables		: Array re-filled each iteration via
 *			`FLIRAx5SetInputVariablesViaUxHwCall`, sized by the caller.
 *	@param	outputVariables		: Array of output-variable results to fill.
 *	@param	monteCarloOutputSamples	: Array of `numberOfMonteCarloIterations` doubles, filled with samples.
 *	@return	double			: Returns the calibrated sensor output of the last iteration.
 */
double
FLIRAx5MonteCarlo(
	size_t      numberOfMonteCarloIterations,
	double      countValueToOverrideDefaultDistribution,
	double *    inputVariables,
	double *    outputVariables,
	double *    monteCarloOutputSamples);
