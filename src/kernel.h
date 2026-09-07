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

#include <math.h>
#include <stddef.h>


#define kCountValueIndicatingNotSetOverride (NAN)

/*
 *	The conversion is defined to be exactly 273.15 with no uncertainty.
 */
#define kAbsoluteZeroKelvinInCelsius (273.15)

/*
 *	These parameter names purposefully mimic the names used in the
 *	reference example by FLIR. As a result, the parameter names do
 *	not follow our usual coding convention.
 */

/*
 *	Object Parameters: Reflected Energy.
 */
#define kFLIRAx5ObjectParameterEmiss    UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)
#define kFLIRAx5ObjectParameterTRefl    UxHwDoubleUniformDist(21.85 - 0.005, 21.85 + 0.005)

/*
 *	Atmospheric Attenuation.
 */
#define kFLIRAx5AtmosphericAttenuationParameterTAtmC    UxHwDoubleUniformDist(21.85 - 0.005, 21.85 + 0.005)
#define kFLIRAx5AtmosphericAttenuationParameterTAtm     (kFLIRAx5AtmosphericAttenuationParameterTAtmC + kAbsoluteZeroKelvinInCelsius)
#define kFLIRAx5AtmosphericAttenuationParameterHumidity UxHwDoubleUniformDist(0.0 / 100)
#define kFLIRAx5AtmosphericAttenuationParameterTau      UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)

/*
 *	External Optics.
 */
#define kFLIRAx5ExternalOpticsParameterTExtOptics               (20)
#define kFLIRAx5ExternalOpticsParameterTransmissionExtOptics    UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)

/*
 *	Camera Calibration Parameters. According to FLIR, these
 *	depend on individual cameras and temperature range situations,
 *	with the values below being for an FLIR Ax5 camera.
 */
#define kFLIRAx5CameraAx5CalibrationParameterR  (16556)
#define kFLIRAx5CameraAx5CalibrationParameterB  UxHwDoubleUniformDist(1428.0 - 0.05, 1428.0 + 0.05)
#define kFLIRAx5CameraAx5CalibrationParameterF  UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)
#define kFLIRAx5CameraAx5CalibrationParameterJ1 UxHwDoubleUniformDist(22.5916 - 0.00005, 22.5916 + 0.00005)
#define kFLIRAx5CameraAx5CalibrationParameterJ0 UxHwDoubleUniformDist(89.796 - 0.0005, 89.796 + 0.0005)

#define kDefaultInputVariableIndexSensorCountsDistLow   (30000)
#define kDefaultInputVariableIndexSensorCountsDistHigh  (30100)

/*
 *	Input Variables:
 *		kInputVariableIndexSensorCounts	: Bolometer sensor counts
 */
typedef enum
{
	kFLIRAx5InputVariableIndexSensorCounts = 0,
	kFLIRAx5InputVariableIndexMax,
} FLIRAx5InputVariableIndex;

/*
 *	Output Variables:
 *		kOutputVariableIndexCalibratedSensorOutput	: Calibrated Sensor output (in units)
 */
typedef enum
{
	kFLIRAx5OutputVariableIndexCalibratedSensorOutput = 0,
	kFLIRAx5OutputVariableIndexMax,
} FLIRAx5OutputVariableIndex;

/**
 *	@brief	Sensor calibration routine. Computes the calibrated temperature output
 *		from raw bolometer counts using the FLIR Ax5 conversion formula.
 *
 *	@param	countValueToOverrideDefaultDistribution	: Scalar
 *					  override for the sensor counts input, or
 *					  `NaN` (`kCountValueIndicatingNotSetOverride`)
 *					  to use the value in `inputVariables` instead.
 *	@param	inputVariables	: The array of input variables used in the calculation.
 *	@param	outputVariables	: An array of output variables. Writes the result to
 *					  `outputVariables[kOutputVariableIndexCalibratedSensorOutput]`.
 *	@return	double			: Returns the distributional value calculated.
 */
double
FLIRAx5_calculateOutput(double countValueToOverrideDefaultDistribution, double *  inputVariables, double *  outputVariables);

/**
 *	@brief	Populate `inputVariables[kFLIRAx5InputVariableIndexSensorCounts]` with a
 *		sample of the raw bolometer sensor counts, drawn from
 *		`UxHwDoubleUniformDist(kDefaultInputVariableIndexSensorCountsDistLow,
 *		kDefaultInputVariableIndexSensorCountsDistHigh)`.
 *
 *		This helper is shared by both the UxHw-mode and Monte Carlo-mode
 *		kernels (`FLIRAx5UxHw` in `flir-ax5-uxhw.c` and `FLIRAx5MonteCarlo`
 *		in `flir-ax5-monte-carlo.c`): in both cases the input is generated
 *		by the same `UxHwDoubleUniformDist` call, which either returns a
 *		full distribution (real Signaloid hardware) or a single
 *		pseudo-random sample per call (the native compat build),
 *		depending on the execution backend.
 *
 *	@param	inputVariables	: Array of size `kFLIRAx5InputVariableIndexMax` to fill.
 */
void
FLIRAx5SetInputVariablesViaUxHwCall(double * inputVariables);

/**
 *	@brief	UxHw-mode calculation kernel. Sizes and owns the
 *		`inputVariables` array (dimensioned by
 *		`kFLIRAx5InputVariableIndexMax`) and dispatches to the purely
 *		computational `FLIRAx5UxHw` (in `flir-ax5-uxhw.c`), which reads no
 *		`kFLIRAx5*` config macros.
 *
 *	@param	countValueToOverrideDefaultDistribution	: Scalar
 *					  override for the sensor counts input, or
 *					  `kCountValueIndicatingNotSetOverride` to use
 *					  the sampled value instead.
 *	@param	outputVariables		: Array of size `kFLIRAx5OutputVariableIndexMax` to fill.
 *	@param	monteCarloOutputSamples	: Single-element array for the distributional result.
 *	@return	double			: Returns the calibrated sensor output.
 */
double
FLIRAx5CalculateOutputUxHw(
	double      countValueToOverrideDefaultDistribution,
	double *    outputVariables,
	double *    monteCarloOutputSamples);

/**
 *	@brief	Monte Carlo calculation kernel. Sizes and owns the
 *		`inputVariables` array (dimensioned by
 *		`kFLIRAx5InputVariableIndexMax`) and dispatches to the purely
 *		computational `FLIRAx5MonteCarlo` (in `flir-ax5-monte-carlo.c`),
 *		which reads no `kFLIRAx5*` config macros.
 *
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *	@param	countValueToOverrideDefaultDistribution	: Scalar
 *					  override for the sensor counts input, or
 *					  `kCountValueIndicatingNotSetOverride` to use
 *					  the per-iteration sampled value instead.
 *	@param	outputVariables		: Array of size `kFLIRAx5OutputVariableIndexMax` to fill.
 *	@param	monteCarloOutputSamples	: Array of `numberOfMonteCarloIterations` doubles, filled with samples.
 *	@return	double			: Returns the calibrated sensor output of the last iteration.
 */
double
FLIRAx5CalculateOutputMonteCarlo(
	size_t      numberOfMonteCarloIterations,
	double      countValueToOverrideDefaultDistribution,
	double *    outputVariables,
	double *    monteCarloOutputSamples);
