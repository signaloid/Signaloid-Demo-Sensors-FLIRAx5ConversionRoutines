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

/**
 *	@brief	UxHw-mode calculation kernel. Performs a single distributional
 *		evaluation of the FLIR Ax5 conversion routine: samples the
 *		sensor's distributional input parameters once via
 *		`FLIRAx5SetInputVariablesViaUxHwCall` and calls
 *		`FLIRAx5_calculateOutput` once. Writes the result to
 *		`outputVariables[0]` and to `monteCarloOutputSamples[0]`.
 *
 *		Purely computational: no dispatch-layer struct parameter and no
 *		config macro reads. The caller (`FLIRAx5CalculateOutputUxHw` in
 *		`kernel.c`) sizes `inputVariables`.
 *
 *	@param	countValueToOverrideDefaultDistribution	: Scalar
 *			override for the sensor counts input, or `NaN` to use
 *			the sampled value in `inputVariables` instead.
 *	@param	inputVariables		: Array to fill via
 *			`FLIRAx5SetInputVariablesViaUxHwCall`, sized by the caller.
 *	@param	outputVariables		: Array to fill with per-output results.
 *	@param	monteCarloOutputSamples	: Single-element array for the distributional result.
 *	@return	double			: Returns the calibrated sensor output.
 */
double
FLIRAx5UxHw(
	double      countValueToOverrideDefaultDistribution,
	double *    inputVariables,
	double *    outputVariables,
	double *    monteCarloOutputSamples);
