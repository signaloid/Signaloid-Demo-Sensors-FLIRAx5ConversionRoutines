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

#include <math.h>
#include <uxhw.h>
#include "kernel.h"


double
FLIRAx5_calculateOutput(double countValueReadFromArgvToOverrideDefaultDistribution, double *  inputVariables, double *  outputVariables)
{
	/*
	 *	These parameter names purposefully mimic the names used in the
	 *	reference example by FLIR. As a result, the parameter names do
	 *	not follow our usual coding convention.
	 */
	double  K1;
	double  K2;
	double  r1;
	double  r2;
	double  r3;
	double  signal;
	double  calibratedValue;
	double  counts;

	if (isnan(countValueReadFromArgvToOverrideDefaultDistribution))
	{
		counts = inputVariables[kFLIRAx5InputVariableIndexSensorCounts];
	}
	else
	{
		counts = countValueReadFromArgvToOverrideDefaultDistribution;
	}

	K1 = 1 /
		 (
		kFLIRAx5AtmosphericAttenuationParameterTau *
		kFLIRAx5ObjectParameterEmiss *
		kFLIRAx5ExternalOpticsParameterTransmissionExtOptics
	     );

	/*
	 *	Pseudo radiance of the reflected environment
	 */
	r1 = ((1 - kFLIRAx5ObjectParameterEmiss) / kFLIRAx5ObjectParameterEmiss) *
		 (
		kFLIRAx5CameraAx5CalibrationParameterR /
		(
			pow(M_E, kFLIRAx5CameraAx5CalibrationParameterB / kFLIRAx5ObjectParameterTRefl) -
			kFLIRAx5CameraAx5CalibrationParameterF
		)
	     );

	/*
	 *	Pseudo radiance of the atmosphere
	 */
	r2 = (
		(1 - kFLIRAx5AtmosphericAttenuationParameterTau) /
		(kFLIRAx5ObjectParameterEmiss * kFLIRAx5AtmosphericAttenuationParameterTau)
	     ) *
		 (
		kFLIRAx5CameraAx5CalibrationParameterR /
		(
			pow(M_E, kFLIRAx5CameraAx5CalibrationParameterB / kFLIRAx5AtmosphericAttenuationParameterTAtm) -
			kFLIRAx5CameraAx5CalibrationParameterF
		)
	     );

	/*
	 *	Pseudo radiance of the external optics
	 */
	r3 = (
		(1 - kFLIRAx5ExternalOpticsParameterTransmissionExtOptics) /
		(
			kFLIRAx5ObjectParameterEmiss *
			kFLIRAx5AtmosphericAttenuationParameterTau *
			kFLIRAx5ExternalOpticsParameterTransmissionExtOptics
		)
	     ) *
		 (
		kFLIRAx5CameraAx5CalibrationParameterR /
		(
			pow(M_E, kFLIRAx5CameraAx5CalibrationParameterB / kFLIRAx5ExternalOpticsParameterTExtOptics) -
			kFLIRAx5CameraAx5CalibrationParameterF
		)
	     );

	K2              = r1 + r2 + r3;
	signal          = (counts - kFLIRAx5CameraAx5CalibrationParameterJ0) / kFLIRAx5CameraAx5CalibrationParameterJ1;
	calibratedValue = (
		kFLIRAx5CameraAx5CalibrationParameterB /
		log(
			kFLIRAx5CameraAx5CalibrationParameterR /
			((K1 * signal) - K2) + kFLIRAx5CameraAx5CalibrationParameterF
		)
	                  ) - kAbsoluteZeroKelvinInCelsius;

	outputVariables[kFLIRAx5OutputVariableIndexCalibratedSensorOutput] = calibratedValue;

	return calibratedValue;
}
