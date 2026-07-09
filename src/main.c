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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <uxhw.h>
#include "utilities.h"
#include "kernel.h"

/**
 *	@brief  Sets the Input Variables via call to UxHw Parametric function.
 *
 *	@param  inputVariables	: An array of double values, where the function writes the distributional data.
 */
static void
setInputVariablesViaUxHwCall(double * inputVariables)
{
	inputVariables[kFLIRAx5InputVariableIndexSensorCounts] = UxHwDoubleUniformDist(
		kDefaultInputVariableIndexSensorCountsDistLow,
		kDefaultInputVariableIndexSensorCountsDistHigh
	);

	return;
}

int
main(int argc, char *  argv[])
{
	CommandLineArguments arguments = { 0 };

	double          calibratedSensorOutput;
	double *        monteCarloOutputSamples = NULL;
	clock_t         start;
	clock_t         end;
	double          cpuTimeUsedSeconds;
	double          inputVariables[kFLIRAx5InputVariableIndexMax];
	double          outputVariables[kFLIRAx5OutputVariableIndexMax];
	const char *    outputVariableNames[kFLIRAx5OutputVariableIndexMax] = {
		"Calibrated FLIR Ax5 Temperature Output"
	};
	const char *    outputVariableDescriptions[kFLIRAx5OutputVariableIndexMax] = {
		"Calibrated FLIR Ax5 sensor output in Kelvin."
	};
	const char *    applicationDescription = "FLIR Lepton Ax5 Sensor Calibration";
	MeanAndVariance meanAndVariance;

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
			__LINE__
		);
	}

	/*
	 *	Start timing.
	 */
	if (arguments.common.isTimingEnabled)
	{
		start = clock();
	}

	for (size_t ii = 0; ii < arguments.common.numberOfMonteCarloIterations; ii++)
	{
		/*
		 *	Set input distribution values, inside the main computation
		 *	loop, so that it can also generate samples in the native
		 *	Monte Carlo Execution Mode.
		 */
		setInputVariablesViaUxHwCall(inputVariables);

		calibratedSensorOutput = FLIRAx5_calculateOutput(arguments.countValueReadFromArgvToOverrideDefaultDistribution, inputVariables, outputVariables);

		/*
		 *	For this application, calibratedSensorOutput is the item we track.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[ii] = calibratedSensorOutput;
		}
	}

	/*
	 *	If not doing Laplace version, then approximate the cost of the third phase of
	 *	Monte Carlo (post-processing), by calculating the mean and variance.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		meanAndVariance         = calculateMeanAndVarianceOfDoubleSamples(monteCarloOutputSamples, arguments.common.numberOfMonteCarloIterations);
		calibratedSensorOutput  = meanAndVariance.mean;
	}

	/*
	 *	Stop timing.
	 */
	if (arguments.common.isTimingEnabled)
	{
		end                 = clock();
		cpuTimeUsedSeconds  = ((double) (end - start)) / CLOCKS_PER_SEC;
	}

	/*
	 *	Print the results (either in JSON or standard output format).
	 */
	if (arguments.common.isOutputJSONMode)
	{
		printJSONFormattedOutput(
			&arguments.common,
			monteCarloOutputSamples,
			outputVariables,
			outputVariableNames,
			kFLIRAx5OutputVariableIndexMax,
			applicationDescription
		);
	}
	else
	{
		printHumanConsumableOutput(
			&arguments.common,
			kFLIRAx5OutputVariableIndexMax,
			outputVariables,
			outputVariableNames,
			outputVariableDescriptions,
			monteCarloOutputSamples
		);
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
				outputVariables,
				outputVariableNames,
				kFLIRAx5OutputVariableIndexMax
		))
		{
			return kCommonConstantReturnTypeError;
		}
	}

	/*
	 *	Save Monte carlo outputs in an output file.
	 *	Free dynamically-allocated memory.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples, (uint64_t) (cpuTimeUsedSeconds * 1000000),
			arguments.common.numberOfMonteCarloIterations
		);

		free(monteCarloOutputSamples);
	}

	return 0;
}
