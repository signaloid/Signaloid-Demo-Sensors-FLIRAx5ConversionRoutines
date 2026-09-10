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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include <uxhw.h>
#include <string.h>
#include <inttypes.h>
#include "utilities.h"
#include "kernel.h"

#ifdef NO_OS_AVAILABLE

void
returnZeroNoOS(void);
#endif

int
main(int argc, char *  argv[])
{
	CommandLineArguments arguments = { 0 };

	double                      calibratedSensorOutput;
	double *                    monteCarloOutputSamples = NULL;
	clock_t                     start;
	clock_t                     end;
	double                      cpuTimeUsedSeconds;
	double                      outputVariables[kFLIRAx5OutputVariableIndexMax];
	const char *                outputVariableNames[kFLIRAx5OutputVariableIndexMax] = {
		"Calibrated FLIR Ax5 Temperature Output"
	};
	const char *                outputVariableDescriptions[kFLIRAx5OutputVariableIndexMax] = {
		"Calibrated FLIR Ax5 sensor output in Kelvin."
	};
	kOutputVariableTypeIndex    outputVariableTypes[kFLIRAx5OutputVariableIndexMax] = {
		kOutputVariableTypeDistribution
	};
	const char *                applicationDescription = "FLIR Lepton Ax5 Sensor Calibration";
	MeanAndVariance             meanAndVariance;

	/*
	 *	Get command line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments))
	{
		return kCommonConstantReturnTypeError;
	}

	/*
	 *	MonteCarlo output samples are used even in the UxHw use case to store
	 *	the result of the single distributional evaluation.
	 */
	monteCarloOutputSamples = (double *) checkedMalloc(
		(arguments.common.numberOfMonteCarloIterations > 0 ? arguments.common.numberOfMonteCarloIterations : 1) * sizeof(double),
		__FILE__,
		__LINE__
	);

	/*
	 *	Start timing.
	 */
	if (arguments.common.isTimingEnabled)
	{
		start = clock();
	}

	/*
	 *	Dispatch to the mode-specific kernel. The Monte Carlo loop lives
	 *	inside `FLIRAx5CalculateOutputMonteCarlo` while UxHw mode runs a single
	 *	distributional evaluation inside `FLIRAx5CalculateOutputUxHw`. Both
	 *	take the fields they need as scalars, so this is the only place that
	 *	unpacks `CommandLineArguments` for the kernel.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		calibratedSensorOutput = FLIRAx5CalculateOutputMonteCarlo(
			arguments.common.numberOfMonteCarloIterations,
			arguments.countValueReadFromArgvToOverrideDefaultDistribution,
			outputVariables,
			monteCarloOutputSamples
		);

		/*
		 *	If not doing UxHw version, then approximate the cost of the third phase of
		 *	Monte Carlo (post-processing), by calculating the mean and variance.
		 */
		meanAndVariance         = calculateMeanAndVarianceOfDoubleSamples(monteCarloOutputSamples, arguments.common.numberOfMonteCarloIterations);
		calibratedSensorOutput  = meanAndVariance.mean;
	}
	else
	{
		calibratedSensorOutput = FLIRAx5CalculateOutputUxHw(
			arguments.countValueReadFromArgvToOverrideDefaultDistribution,
			outputVariables,
			monteCarloOutputSamples
		);
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
		printf("\nCPU time used: %" SignaloidParticleModifier "lf seconds\n", cpuTimeUsedSeconds);
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
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples, (uint64_t) (cpuTimeUsedSeconds * 1000000),
			arguments.common.numberOfMonteCarloIterations
		);
	}

	/*
	 *	Free dynamically-allocated memory. `monteCarloOutputSamples` is
	 *	always allocated (even in UxHw mode, to hold the single
	 *	distributional result), so it is always freed here.
	 */
	free(monteCarloOutputSamples);

	/*
	 *	`calibratedSensorOutput` and `outputVariableTypes` are not read
	 *	past this point: `calibratedSensorOutput`'s only purpose in Monte
	 *	Carlo mode is to approximate the cost of the mean/variance
	 *	post-processing phase (see above), and `outputVariableTypes` is
	 *	kept for structural consistency with other demos even though this
	 *	sensor has a single, always-distributional output with no scalar
	 *	output-select handling to drive.
	 */
	(void) calibratedSensorOutput;
	(void) outputVariableTypes;

#ifdef NO_OS_AVAILABLE
	returnZeroNoOS();
#else

	return 0;

#endif
}
