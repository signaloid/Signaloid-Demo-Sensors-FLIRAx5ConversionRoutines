/*
 *	Copyright (c) 2026, Signaloid.
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
#define kFLIRAx5ObjectParameterEmiss   UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)
#define kFLIRAx5ObjectParameterTRefl   UxHwDoubleUniformDist(21.85 - 0.005, 21.85 + 0.005)

/*
 *	Atmospheric Attenuation.
 */
#define kFLIRAx5AtmosphericAttenuationParameterTAtmC       UxHwDoubleUniformDist(21.85 - 0.005, 21.85 + 0.005)
#define kFLIRAx5AtmosphericAttenuationParameterTAtm        (kFLIRAx5AtmosphericAttenuationParameterTAtmC + kAbsoluteZeroKelvinInCelsius)
#define kFLIRAx5AtmosphericAttenuationParameterHumidity    UxHwDoubleUniformDist(0.0 / 100)
#define kFLIRAx5AtmosphericAttenuationParameterTau         UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)

/*
 *	External Optics.
 */
#define kFLIRAx5ExternalOpticsParameterTExtOptics              (20)
#define kFLIRAx5ExternalOpticsParameterTransmissionExtOptics   UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)

/*
 *	Camera Calibration Parameters. According to FLIR, these
 *	depend on individual cameras and temperature range situations,
 *	with the values below being for an FLIR Ax5 camera.
 */
#define kFLIRAx5CameraAx5CalibrationParameterR     (16556)
#define kFLIRAx5CameraAx5CalibrationParameterB     UxHwDoubleUniformDist(1428.0 - 0.05, 1428.0 + 0.05)
#define kFLIRAx5CameraAx5CalibrationParameterF     UxHwDoubleUniformDist(1.0 - 0.05, 1.0 + 0.05)
#define kFLIRAx5CameraAx5CalibrationParameterJ1    UxHwDoubleUniformDist(22.5916 - 0.00005, 22.5916 + 0.00005)
#define kFLIRAx5CameraAx5CalibrationParameterJ0    UxHwDoubleUniformDist(89.796 - 0.0005, 89.796 + 0.0005)

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
 *	@param	arguments		: Pointer to command line arguments struct.
 *	@param	inputVariables	: The array of input variables used in the calculation.
 *	@param	outputVariables	: An array of output variables. Writes the result to
 *					  `outputVariables[kOutputVariableIndexCalibratedSensorOutput]`.
 *	@return	double			: Returns the distributional value calculated.
 */
double
FLIRAx5_calculateOutput(double countValueReadFromArgvToOverrideDefaultDistribution, double *  inputVariables, double *  outputVariables);
