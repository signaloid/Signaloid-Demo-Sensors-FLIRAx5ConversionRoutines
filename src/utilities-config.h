/*
 *	Copyright (c) 2024, Signaloid.
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

#define	kCountValueIndicatingNotSetOverride			(NAN)

/*
 *	The conversion is defined to be exactly 273.15 with no uncertainty.
 */
#define	kAbsoluteZeroKelvinInCelsius				(273.15)

/*
 *	These parameter names purposefully mimic the names used in the
 *	reference example by FLIR. As a result, the parameter names do
 *	not follow our usual coding convention.
 */

/*
 *	Object Parameters: Reflected Energy.
 */
#define	kFLIRobjectParameterEmiss				UxHwDoubleUniformDist(1.0 - 0.05,		1.0 + 0.05)
#define	kFLIRobjectParameterTRefl				UxHwDoubleUniformDist(21.85 - 0.005,		21.85 + 0.005)

/*
 *	Atmospheric Attenuation.
 */
#define	kFLIRatmosphericAttenuationParameterTAtmC		UxHwDoubleUniformDist(21.85 - 0.005,		21.85 + 0.005)
#define	kFLIRatmosphericAttenuationParameterTAtm		(kFLIRatmosphericAttenuationParameterTAtmC + kAbsoluteZeroKelvinInCelsius)
#define	kFLIRatmosphericAttenuationParameterHumidity		UxHwDoubleUniformDist(0.0/100)
#define	kFLIRatmosphericAttenuationParameterTau			UxHwDoubleUniformDist(1.0 - 0.05,		1.0 + 0.05)

/*
 *	External Optics.
 */
#define	kFLIRexternalOpticsParameterTExtOptics			(20)
#define	kFLIRexternalOpticsParameterTransmissionExtOptics	UxHwDoubleUniformDist(1.0 - 0.05,		1.0 + 0.05)

/*
 *	Camera Calibration Parameters. According to FLIR, these
 *	depend on individual cameras and temperature range situations,
 *	with the values below being for an FLIR Ax5 camera.
 */
#define	kFLIRcameraAx5CalibrationParameterR			(16556)
#define	kFLIRcameraAx5CalibrationParameterB			UxHwDoubleUniformDist(1428.0 - 0.05,		1428.0 + 0.05)
#define	kFLIRcameraAx5CalibrationParameterF			UxHwDoubleUniformDist(1.0 - 0.05,		1.0 + 0.05)
#define	kFLIRcameraAx5CalibrationParameterJ1			UxHwDoubleUniformDist(22.5916 - 0.00005,	22.5916 + 0.00005)
#define	kFLIRcameraAx5CalibrationParameterJ0			UxHwDoubleUniformDist(89.796 - 0.0005,		89.796 + 0.0005)

/*
 *	Input Distributions:
 *		kInputSensorCounts	: Bolometer sensor counts
 */
typedef enum
{
	kInputDistributionIndexSensorCounts			= 0,
	kInputDistributionIndexMax,
} InputDistributionIndex;

/*
 *	Output Distributions:
 *		kOutputDistributionIndexCalibratedSensorOutput	: Calibrated Sensor output (in units)
 */
typedef enum
{
	kOutputDistributionIndexCalibratedSensorOutput		= 0,
	kOutputDistributionIndexMax,
} OutputDistributionIndex;

#define kDefaultInputDistributionIndexSensorCountsDistLow	(30000)
#define kDefaultInputDistributionIndexSensorCountsDistHigh	(30100)
