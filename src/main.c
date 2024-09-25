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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <uxhw.h>
#include "utilities.h"

/**
 *	@brief  Sets the Input Distributions via call to UxHw Parametric function.
 *
 *	@param  inputDistributions	: An array of double values, where the function writes,
 *					the distributional data.
 */
static void
setInputDistributionsViaUxHwCall(double *  inputDistributions)
{
	inputDistributions[kInputDistributionIndexSensorCounts] = UxHwDoubleUniformDist(
									kDefaultInputDistributionIndexSensorCountsDistLow,
									kDefaultInputDistributionIndexSensorCountsDistHigh);

	return;
}

/**
 *	@brief  Sensor calibration routine.
 *
 *	@param  arguments		: Pointer to command line arguments struct.
 *	@param  inputDistributions	: The array of input distributions used in the calculation.
 * 	@param  outputDistributions	: An array of of output distributions. Writes the result to `outputDistributions[outputSelectValue]`.
 *
 *	@return	double			: Returns the distributional value calculated.
 */
static double
calculateSensorOutput(CommandLineArguments *  arguments, double *  inputDistributions, double *  outputDistributions)
{
	/*
	 *	These parameter names purposefully mimic the names used in the
	 *	reference example by FLIR. As a result, the parameter names do
	 *	not follow our usual coding convention.
	 */
	double	K1;
	double	K2;
	double	r1;
	double	r2;
	double	r3;
	double	signal;
	double	calibratedValue;
	double	counts;

	if (isnan(arguments->countValueReadFromArgvToOverrideDefaultDistribution))
	{
		counts = inputDistributions[kInputDistributionIndexSensorCounts];
	}
	else
	{
		counts = arguments->countValueReadFromArgvToOverrideDefaultDistribution;
	}

	K1	=	1 /
			(
				kFLIRatmosphericAttenuationParameterTau *
				kFLIRobjectParameterEmiss *
				kFLIRexternalOpticsParameterTransmissionExtOptics
			);

	/*
	 *	Pseudo radiance of the reflected environment
	 */
	r1	=	((1 - kFLIRobjectParameterEmiss)/kFLIRobjectParameterEmiss) * 
			(
				kFLIRcameraAx5CalibrationParameterR /
				(
					pow(M_E, kFLIRcameraAx5CalibrationParameterB/kFLIRobjectParameterTRefl) -
					kFLIRcameraAx5CalibrationParameterF
				)
			);

	/*
	 *	Pseudo radiance of the atmosphere
	 */
	r2	=	(
				(1 - kFLIRatmosphericAttenuationParameterTau) /
				(kFLIRobjectParameterEmiss * kFLIRatmosphericAttenuationParameterTau)
			) *
			(
				kFLIRcameraAx5CalibrationParameterR /
				(
					pow(M_E, kFLIRcameraAx5CalibrationParameterB/kFLIRatmosphericAttenuationParameterTAtm) -
					kFLIRcameraAx5CalibrationParameterF
				)
			);

	/*
	 *	Pseudo radiance of the external optics
	 */
	r3	=	(
				(1 - kFLIRexternalOpticsParameterTransmissionExtOptics) /
				(
					kFLIRobjectParameterEmiss *
					kFLIRatmosphericAttenuationParameterTau *
					kFLIRexternalOpticsParameterTransmissionExtOptics
				)
			) *
			(
				kFLIRcameraAx5CalibrationParameterR /
				(
					pow(M_E, kFLIRcameraAx5CalibrationParameterB/kFLIRexternalOpticsParameterTExtOptics) -
					kFLIRcameraAx5CalibrationParameterF
				)
			);

	K2	=	r1 + r2 + r3;
	signal	=	(counts - kFLIRcameraAx5CalibrationParameterJ0) / kFLIRcameraAx5CalibrationParameterJ1;
	calibratedValue	=	(
					kFLIRcameraAx5CalibrationParameterB /
					log(
						kFLIRcameraAx5CalibrationParameterR /
						((K1 * signal) - K2) + kFLIRcameraAx5CalibrationParameterF
					)
				) - kAbsoluteZeroKelvinInCelsius;

	outputDistributions[kOutputDistributionIndexCalibratedSensorOutput] = calibratedValue;

	return	calibratedValue;
}

int
main(int argc, char *  argv[])
{
	CommandLineArguments	arguments = {0};

	double			calibratedSensorOutput;
	double *		monteCarloOutputSamples = NULL;
	clock_t			start;
	clock_t			end;
	double			cpuTimeUsedSeconds;
	double			inputDistributions[kInputDistributionIndexMax];
	double			outputDistributions[kOutputDistributionIndexMax];
	const char *		outputVariableNames[kOutputDistributionIndexMax] =
				{
					"Calibrated FLIR Ax5 Temperature Output",
				};
	const char *		unitsOfMeasurement[kOutputDistributionIndexMax] =
				{
					"Kelvin",
				};
	MeanAndVariance		meanAndVariance;

	/*
	 *	Get command line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments))
	{
		return kCommonConstantReturnTypeError;
	}

	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples = (double *) checkedMalloc(
							arguments.common.numberOfMonteCarloIterations * sizeof(double),
							__FILE__,
							__LINE__);
	}

	/*
	 *	Start timing.
	 */
	if (arguments.common.isTimingEnabled || arguments.common.isBenchmarkingMode)
	{
		start = clock();
	}

	for (size_t i = 0; i < arguments.common.numberOfMonteCarloIterations; i++)
	{
		/*
		 *	Set input distribution values, inside the main computation
		 *	loop, so that it can also generate samples in the native
		 *	Monte Carlo Execution Mode.
		 */
		setInputDistributionsViaUxHwCall(inputDistributions);

		calibratedSensorOutput = calculateSensorOutput(&arguments, inputDistributions, outputDistributions);

		/*
		 *	For this application, calibratedSensorOutput is the item we track.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[i] = calibratedSensorOutput;
		}
	}

	/*
	 *	If not doing Laplace version, then approximate the cost of the third phase of
	 *	Monte Carlo (post-processing), by calculating the mean and variance.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		meanAndVariance = calculateMeanAndVarianceOfDoubleSamples(monteCarloOutputSamples, arguments.common.numberOfMonteCarloIterations);
		calibratedSensorOutput = meanAndVariance.mean;
	}

	/*
	 *	Stop timing.
	 */
	if (arguments.common.isTimingEnabled || arguments.common.isBenchmarkingMode)
	{
		end = clock();
		cpuTimeUsedSeconds = ((double)(end - start)) / CLOCKS_PER_SEC;
	}

	if (arguments.common.isBenchmarkingMode)
	{
		/*
		 *	In benchmarking mode, we print:
		 *		(1) single result (for calculating Wasserstein distance to reference)
		 *		(2) time in microseconds (benchmarking setup expects cpu time in microseconds)
		 */
		printf("%lf %" PRIu64 "\n", calibratedSensorOutput, (uint64_t)(cpuTimeUsedSeconds*1000000));
	}
	else
	{
		/*
		 *	Print the results (either in JSON or standard output format).
		 */
		if (!arguments.common.isOutputJSONMode)
		{
				printCalibratedValueAndProbabilities(
					calibratedSensorOutput,
					outputVariableNames[kOutputDistributionIndexCalibratedSensorOutput],
					unitsOfMeasurement[kOutputDistributionIndexCalibratedSensorOutput]);
		}
		else
		{
			printJSONFormattedOutput(
				&arguments,
				&outputDistributions[kOutputDistributionIndexCalibratedSensorOutput],
				monteCarloOutputSamples,
				outputVariableNames[kOutputDistributionIndexCalibratedSensorOutput]);
		}

		/*
		 *	Print timing result.
		 */
		if (arguments.common.isTimingEnabled)
		{
			printf("\nCPU time used: %lf seconds\n", cpuTimeUsedSeconds);
		}

		/*
		 *	Write output data.
		 */
		if (arguments.common.isWriteToFileEnabled)
		{
			if (writeOutputDoubleDistributionsToCSV(
				arguments.common.outputFilePath,
				outputDistributions,
				outputVariableNames,
				kOutputDistributionIndexMax))
			{
				return kCommonConstantReturnTypeError;
			}
		}
	}

	/*
	 *	Save Monte carlo outputs in an output file.
	 *	Free dynamically-allocated memory.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(monteCarloOutputSamples, (uint64_t)(cpuTimeUsedSeconds*1000000), arguments.common.numberOfMonteCarloIterations);

		free(monteCarloOutputSamples);
	}

	return 0;
}
