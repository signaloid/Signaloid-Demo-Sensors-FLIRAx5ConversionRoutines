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

#pragma once

#include "common.h"
#include "utilities-config.h"

typedef struct
{
	CommonCommandLineArguments	common;
	double				countValueReadFromArgvToOverrideDefaultDistribution;
} CommandLineArguments;

/**
 *	@brief	Print out command line usage.
 */
void	printUsage(void);

/**
 *	@brief	Get command line arguments.
 *
 *	@param	argc		: argument count from main().
 *	@param	argv		: argument vector from main().
 *	@param	arguments	: Pointer to struct to store arguments.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful,
 *				   else `kCommonConstantReturnTypeError`.
 */
CommonConstantReturnType getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments);

/**
 *	@brief  Prints the output of the evaluation in a human-readable form.
 *
 *	@param  calibratedSensorOutput	: A single result of the evaluation. Calculates useful statistics from it.
 *	@param  variableDescription	: A string decribing the mode of the sensor it prints.
 *	@param  unitsOfMeasurement	: A string decribing the units of measurement of the value it prints.
 */
void	printCalibratedValueAndProbabilities(double calibratedSensorOutput, const char *  variableDescription, const char *  unitsOfMeasurement);

/**
 *	@brief  Prints output distributions in JSON format. Based on command-line arguments will either print
 *		a single value or all values stored in `outputDistributions`.
 *
 *	@param  arguments			: The command-line arguments, specifying which outputs will be printed.
 *	@param  outputVariable 			: A pointer to the distribution to print.
 *	@param  monteCarloOutputSamples		: The array of data samples of Monte Carlo.
 *	@param  variableDescription		: A string containing a description of the variable printed.
 */
void	printJSONFormattedOutput(
		CommandLineArguments *	arguments,
		double *		outputVariable,
		double *		monteCarloOutputSamples,
		const char *		variableDescription);
