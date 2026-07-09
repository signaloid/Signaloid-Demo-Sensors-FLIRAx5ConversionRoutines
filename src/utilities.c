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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <uxhw.h>
#include "common.h"
#include "utilities.h"
#include "kernel.h"

void
printUsage(void)
{
	fprintf(stderr, "FLIR microbolometer array radiometric to temperature conversion routines.\n");
	fprintf(stderr, "Usage: Valid command-line arguments are:\n");
	fprintf(
		stderr,
		"\t[-o, --output <Path to output CSV file : str>] (Specify the output file.)\n"
		"\t[-S, --select-output <output : int>] (Compute 0-indexed output, by default 0.)\n"
		"\t[-M, --multiple-executions <Number of executions : int (Default: 1)>] (Repeated execute kernel for benchmarking.)\n"
		"\t[-T, --time] (Timing mode: Times and prints the timing of the kernel execution.)\n"
		"\t[-b, --benchmarking] (Benchmarking mode: Generate outputs in format for benchmarking.)\n"
		"\t[-j, --json] (Print output in JSON format.)\n"
		"\t[-h, --help] (Display this help message.)\n"
	);
	/*
	 *	Print demo specific options usage
	 */
	fprintf(
		stderr,
		"\t[-sp, --sensor-parameter <particle value used to override default distribution for `counts`: double>]\n"
	);
	fprintf(stderr, "\n");

	return;
}

static void
setDefaultCommandLineArguments(CommandLineArguments * arguments)
{
	/*
	 *	Older GCC versions have a bug which gives a spurious warning for the C universal zero
	 *	initializer `{0}`. Any workaround makes the code less portable or prevents the common code
	 *	from adding new fields to the `CommonCommandLineArguments` struct. Therefore, we surpress
	 *	this warning.
	 *
	 *	See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119.
	 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
	*arguments = (CommandLineArguments)
	{
		.common = (CommonCommandLineArguments) { 0 },
	};
#pragma GCC diagnostic pop

	/*
	 *	Initialize any demo-specific command line arguments to default values
	 */
	arguments->countValueReadFromArgvToOverrideDefaultDistribution = kCountValueIndicatingNotSetOverride;

	return;
}

CommonConstantReturnType
getCommandLineArguments(
	int                     argc,
	char *                  argv[],
	CommandLineArguments *  arguments)
{
	const char *    sensorParameterArg      = NULL;
	bool            sensorParameterArgFound = false;
	DemoOption      options[]               = {
		{ .opt = "sp", .optAlternative = "sensor-parameter", .hasArg = true, .foundArg = &sensorParameterArg, .foundOpt = &sensorParameterArgFound },
		{ 0 },
	};

	if (arguments == NULL)
	{
		fprintf(stderr, "Arguments pointer is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	setDefaultCommandLineArguments(arguments);

	/*
	 *	This application example has no application specific arguments.
	 */
	if (parseArgs(argc, argv, &arguments->common, options) != 0)
	{
		fprintf(stderr, "Parsing command line arguments failed\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Process command-line arguments.
	 */
	if (arguments->common.isHelpEnabled)
	{
		printUsage();

		exit(EXIT_SUCCESS);
	}

	/*
	 *	For most of the sensor applications, we do not support reading
	 *	from input files.
	 */
	if (arguments->common.isInputFromFileEnabled)
	{
		fprintf(stderr, "Reading inputs from CSV file is not currently supported\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Write to output file is not supported in MonteCarlo Mode.
	 */
	if (arguments->common.isWriteToFileEnabled && arguments->common.isMonteCarloMode)
	{
		fprintf(stderr, "Writing to output file is not supported in MonteCarlo Mode.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Warning: Verbose mode not supported. Continuing in non-verbose mode.\n");
	}

	/*
	 *	If no output selected from CLA, set the print all value as default.
	 */
	if (!arguments->common.isOutputSelected)
	{
		arguments->common.outputSelect = kFLIRAx5OutputVariableIndexCalibratedSensorOutput;
	}

	/*
	 *	If a single output is selected, we must be in benchmarking mode or Monte Carlo mode.
	 */
	if (arguments->common.outputSelect > kFLIRAx5OutputVariableIndexMax)
	{
		fprintf(
			stderr,
			"Output select value (-S option) is greater than the possible number of outputs: Provided %zd. Max: %d\n",
			arguments->common.outputSelect,
			kFLIRAx5OutputVariableIndexMax
		);
	}
	/*
	 *	When all outputs are selected, we cannot be in benchmarking mode or Monte Carlo mode.
	 */
	else if (arguments->common.outputSelect == kFLIRAx5OutputVariableIndexMax)
	{
		if ((arguments->common.isBenchmarkingMode) || (arguments->common.isMonteCarloMode))
		{
			fprintf(stderr, "Error: Please select a single output when in benchmarking mode or Monte Carlo mode.\n");

			return kCommonConstantReturnTypeError;
		}
	}

	/*
	 *	Handle demo-specific options
	 */
	if (sensorParameterArgFound)
	{
		CommonConstantReturnType ret = parseDoubleChecked(sensorParameterArg, &arguments->countValueReadFromArgvToOverrideDefaultDistribution);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			arguments->countValueReadFromArgvToOverrideDefaultDistribution = NAN;
			fprintf(stderr, "Error: The countValueReadFromArgvToOverrideDefaultDistribution value must be a real number. Setting it to NAN.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}
	}

	return kCommonConstantReturnTypeSuccess;
}
