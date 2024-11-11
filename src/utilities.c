/*
 *	Copyright (c) 2021–2024, Signaloid.
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
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <uxhw.h>
#include "utilities.h"
#include "common.h"

const char *	kDefaultMeasurementsPathPrefix		= "warp-board-002";
const char *	kDefaultCalibrationConstantsPathPrefix	= "BME680-par";

/**
 *	@brief	Set default values for the application-specific command-line arguments.
 *
 *	@param	arguments	: Pointer to the command-line arguments struct.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful, else `kCommonConstantReturnTypeError`.
 */
static CommonConstantReturnType
setDefaultCommandLineArguments(CommandLineArguments *  arguments)
{
	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

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

	*arguments = (CommandLineArguments) {
		.common				= (CommonCommandLineArguments) {0},
		.measurementsPathPrefix		= "",
		.calibrationConstantsPathPrefix	= "",
		.indexForCalibrationParameters	= 0,
		.temperatureRawADCValue		= kBME680ConstantsTemperatureRawADCDefaultValue,
		.pressureRawADCValue		= kBME680ConstantsPressureRawADCDefaultValue,
		.humidityRawADCValue		= kBME680ConstantsHumidityRawADCDefaultValue,
		.useInputADCFiles		= false,
	};
#pragma GCC diagnostic pop

	for (size_t i = 0; i < kInputDistributionIndexMax; i++)
	{
		arguments->isInputSetFromCommandLine[i] = false;
	}

	snprintf(
		arguments->measurementsPathPrefix,
		kCommonConstantMaxCharsPerFilepath,
		"%s",
		(char *) kDefaultMeasurementsPathPrefix
		);

	snprintf(
		arguments->calibrationConstantsPathPrefix,
		kCommonConstantMaxCharsPerFilepath,
		"%s",
		(char *) kDefaultCalibrationConstantsPathPrefix
		);

	return kCommonConstantReturnTypeSuccess;
}

void
printUsage(void)
{
	fprintf(stderr, "Example: BME680 sensor calibration routines - Signaloid version\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: Valid command-line arguments are:\n");
	fprintf(stderr,
		"\t[-o, --output <Path to output CSV file : str>] (Specify the output file.)\n"
		"\t[-S, --select-output <output : int> (Default: 0)] (Compute 0-indexed output.)\n"
		"\t[-M, --multiple-executions <Number of executions : int> (Default: 1)] (Repeated execute kernel for benchmarking.)\n"
		"\t[-T, --time] (Timing mode: Times and prints the timing of the kernel execution.)\n"
		"\t[-v, --verbose] (Verbose mode: Prints extra information about demo execution.)\n"
		"\t[-b, --benchmarking] (Benchmarking mode: Generate outputs in format for benchmarking.)\n"
		"\t[-j, --json] (Print output in JSON format.)\n"
		"\t[-h, --help] (Display this help message.)\n"
		"\t[-m, --measurements-prefix <prefix of input ADC traces files : str> (Default: '%s')]\n"
		"\t[-c, --calibration-constants-prefix <prefix of calibration constants files : str> (Default: '%s')]\n"
		"\t[-n, --calibration-parameter-index <index of calibration parameter: int in [0, 4]> (Default: 0)]\n"
		"\t[-t, --override-temperature-measurement <temperature measurement : str> (Default: '')]\n"
		"\t[-p, --override-pressure-measurement <pressure measurement: str> (Default: '')]\n"
		"\t[-u, --override-humidity-measurement <humidity measurement: str> (Default: '')]\n",
		kDefaultMeasurementsPathPrefix,
		kDefaultCalibrationConstantsPathPrefix);
	fprintf(stderr, "\n");
}

CommonConstantReturnType
getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments)
{
	const char *	measurementsPathPrefix = NULL;
	const char *	calibrationConstantsPathPrefix = NULL;
	const char *	indexForCalibrationParametersArg = NULL;
	const char *	temperatureArg = NULL;
	const char *	pressureArg = NULL;
	const char *	humidityArg = NULL;
	const char	kConstantStringUx[] = "Ux";

	if (arguments == NULL)
	{
		fprintf(stderr, "Error: Source command-line arguments are NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	if (setDefaultCommandLineArguments(arguments) != kCommonConstantReturnTypeSuccess)
	{
		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Specify application-specific command-line arguments.
	 */
	DemoOption	options[] = {
		{ .opt = "m", .optAlternative = "measurements-prefix",			.hasArg = true,	.foundArg = &measurementsPathPrefix,		.foundOpt = NULL },
		{ .opt = "c", .optAlternative = "calibration-constants-prefix",		.hasArg = true,	.foundArg = &calibrationConstantsPathPrefix,	.foundOpt = NULL },
		{ .opt = "n", .optAlternative = "calibration-parameter-index",		.hasArg = true,	.foundArg = &indexForCalibrationParametersArg,	.foundOpt = NULL },
		{ .opt = "t", .optAlternative = "override-temperature-measurement",	.hasArg = true,	.foundArg = &temperatureArg,			.foundOpt = NULL },
		{ .opt = "p", .optAlternative = "override-pressure-measurement",	.hasArg = true,	.foundArg = &pressureArg,			.foundOpt = NULL },
		{ .opt = "u", .optAlternative = "override-humidity-measurement",	.hasArg = true,	.foundArg = &humidityArg,			.foundOpt = NULL },
		{0},
	};

	if (parseArgs(argc, argv, &arguments->common, options) != kCommonConstantReturnTypeSuccess)
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

	if (arguments->common.isInputFromFileEnabled)
	{
		fprintf(stderr, "Error: This application does not support reading a single source file. Use `-m` option to read measurements for ADC files.\n");

		return kCommonConstantReturnTypeError;
	}

	if (!arguments->common.isOutputSelected)
	{
		arguments->common.outputSelect = kOutputDistributionIndexMax;
	}

	/*
	 *	When all outputs are selected, we cannot be in benchmarking mode or Monte Carlo mode.
	 */
	if (arguments->common.outputSelect == kOutputDistributionIndexMax)
	{
		if ((arguments->common.isBenchmarkingMode) || (arguments->common.isMonteCarloMode))
		{
			fprintf(stderr, "Error: Please select a single output when in benchmarking mode or Monte Carlo mode.\n");

			return kCommonConstantReturnTypeError;
		}
	}
	/*
	 *	Selected output cannot be greater than `kOutputDistributionIndexMax`.
	 */
	else if (arguments->common.outputSelect > kOutputDistributionIndexMax)
	{
		fprintf(stderr, "Error: Wrong output selection.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Warning: Verbose mode not supported. Continuing in non-verbose mode.\n");
	}

	/*
	 *	Process application-specific command-line arguments.
	 */
	if (temperatureArg != NULL)
	{
		int ret = parseFloatChecked(temperatureArg, &arguments->temperatureRawADCValue);

		if (arguments->common.isMonteCarloMode)
		{
			if (strstr(temperatureArg, kConstantStringUx) != NULL)
			{
				fprintf(stderr, "Error: Native Monte Carlo is not compatible with Ux strings from command line.\n");

				return kCommonConstantReturnTypeError;
			}
		}

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			arguments->temperatureRawADCValue = NAN;
			fprintf(stderr, "Error: The temperature raw ADC value must be a real number. Setting it to NAN.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->isInputSetFromCommandLine[kInputDistributionIndexForTemperatureRawADCValue] = true;
	}

	if (pressureArg != NULL)
	{
		int ret = parseFloatChecked(pressureArg, &arguments->pressureRawADCValue);

		if (arguments->common.isMonteCarloMode)
		{
			if (strstr(pressureArg, kConstantStringUx) != NULL)
			{
				fprintf(stderr, "Error: Native Monte Carlo is not compatible with Ux strings from command line.\n");

				return kCommonConstantReturnTypeError;
			}
		}

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			arguments->pressureRawADCValue = NAN;
			fprintf(stderr, "Error: The pressure raw ADC value must be a real number. Setting it to NAN.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->isInputSetFromCommandLine[kInputDistributionIndexForPressureRawADCValue] = true;
	}

	if (humidityArg != NULL)
	{
		int ret = parseFloatChecked(humidityArg, &arguments->humidityRawADCValue);

		if (arguments->common.isMonteCarloMode)
		{
			if (strstr(humidityArg, kConstantStringUx) != NULL)
			{
				fprintf(stderr, "Error: Native Monte Carlo is not compatible with Ux strings from command line.\n");

				return kCommonConstantReturnTypeError;
			}
		}

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			arguments->humidityRawADCValue = NAN;
			fprintf(stderr, "Error: The humidity raw ADC value must be a real number. Setting it to NAN.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->isInputSetFromCommandLine[kInputDistributionIndexForHumidityRawADCValue] = true;
	}

	if (calibrationConstantsPathPrefix != NULL)
	{
		int ret = snprintf(arguments->calibrationConstantsPathPrefix, kCommonConstantMaxCharsPerFilepath, "%s", calibrationConstantsPathPrefix);

		if ((ret < 0) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Error: Could not copy calibration constants path prefix from command-line arguments.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
	}

	if (measurementsPathPrefix != NULL)
	{
		int ret = snprintf(arguments->measurementsPathPrefix, kCommonConstantMaxCharsPerFilepath, "%s", measurementsPathPrefix);

		if ((ret < 0) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Error: Could not copy measurements path prefix from command-line arguments.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->useInputADCFiles = true;
	}

	if (indexForCalibrationParametersArg != NULL)
	{
		int indexForCalibrationParameters;
		int ret = parseIntChecked(indexForCalibrationParametersArg, &indexForCalibrationParameters);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The calibration parameter must be an integer.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if ((indexForCalibrationParameters < 0) || (indexForCalibrationParameters > 4))
		{
			fprintf(stderr, "Error: Illegal argument %d for option -n. Should be an integer in [0, 4].\n", indexForCalibrationParameters);
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->indexForCalibrationParameters = indexForCalibrationParameters;
	}

	return kCommonConstantReturnTypeSuccess;
}

CommonConstantReturnType
loadInputs(
	CommandLineArguments *	arguments,
	float *			temperatureParameters,
	float *			pressureParameters,
	float *			humidityParameters,
	float *			temperatureRawADCValue,
	float *			pressureRawADCValue,
	float *			humidityRawADCValue)
{
	char		filename[kCommonConstantMaxCharsPerFilepath];
	const char * 	inputHumidityADCTraceCSVHeader[1] = {"Humidity ADC values"};
	const char * 	inputPressureADCTraceCSVHeader[1] = {"Pressure ADC values"};
	const char * 	inputTemperatureADCTraceCSVHeader[1] = {"Temperature ADC values"};
	int 		ret;

	/*
	 *	Load calibration parameters.
	 *	Default: Load all calibration parameters as particle values using `loadNthFloatFromPath()`.
	 */
	for(int i = 0; i < kBME680ConstantsNumberOfTemperatureParameters; i++)
	{
		ret = snprintf(filename, kCommonConstantMaxCharsPerFilepath, "%s-t%d.csv", arguments->calibrationConstantsPathPrefix, i + 1);

		if ((ret < 1) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Failed to create filename for loading from %s-t%d.csv", arguments->calibrationConstantsPathPrefix, i + 1);

			return kCommonConstantReturnTypeError;
		}

		if (loadNthFloatFromPath(filename, arguments->indexForCalibrationParameters, &temperatureParameters[i]) != kCommonConstantReturnTypeSuccess)
		{
			return kCommonConstantReturnTypeError;
		}
	}

	for(int i = 0; i < kBME680ConstantsNumberOfPressureParameters; i++)
	{
		ret = snprintf(filename, kCommonConstantMaxCharsPerFilepath, "%s-p%d.csv", arguments->calibrationConstantsPathPrefix, i + 1);

		if ((ret < 1) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Failed to create filename for loading from %s-p%d.csv", arguments->calibrationConstantsPathPrefix, i + 1);

			return kCommonConstantReturnTypeError;
		}

		if (loadNthFloatFromPath(filename, arguments->indexForCalibrationParameters, &pressureParameters[i]) != kCommonConstantReturnTypeSuccess)
		{
			return kCommonConstantReturnTypeError;
		}
	}

	for(int i = 0; i < kBME680ConstantsNumberOfHumidityParameters; i++)
	{
		ret = snprintf(filename, kCommonConstantMaxCharsPerFilepath, "%s-h%d.csv", arguments->calibrationConstantsPathPrefix, i + 1);

		if ((ret < 1) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Failed to create filename for loading from %s-h%d.csv", arguments->calibrationConstantsPathPrefix, i + 1);

			return kCommonConstantReturnTypeError;
		}

		if (loadNthFloatFromPath(filename, arguments->indexForCalibrationParameters, &humidityParameters[i]) != kCommonConstantReturnTypeSuccess)
		{
			return kCommonConstantReturnTypeError;
		}
	}

	/*
	 *	Load raw ADC output.
	 */
	if(arguments->useInputADCFiles)
	{
		ret = snprintf(filename, kCommonConstantMaxCharsPerFilepath, "%s-temperature-adc-trace.csv", arguments->measurementsPathPrefix);

		if ((ret < 1) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Failed to create filename for loading from %s-temperature-adc-trace.csv", arguments->measurementsPathPrefix);

			return kCommonConstantReturnTypeError;
		}

		if (readInputFloatDistributionsFromCSV(filename, inputTemperatureADCTraceCSVHeader, temperatureRawADCValue, 1) != kCommonConstantReturnTypeSuccess)
		{
			return kCommonConstantReturnTypeError;
		}

		ret = snprintf(filename, kCommonConstantMaxCharsPerFilepath, "%s-pressure-adc-trace.csv", arguments->measurementsPathPrefix);

		if ((ret < 1) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Failed to create filename for loading from %s-pressure-adc-trace.csv", arguments->measurementsPathPrefix);

			return kCommonConstantReturnTypeError;
		}

		if (readInputFloatDistributionsFromCSV(filename, inputPressureADCTraceCSVHeader, pressureRawADCValue, 1) != kCommonConstantReturnTypeSuccess)
		{
			return kCommonConstantReturnTypeError;
		}

		ret = snprintf(filename, kCommonConstantMaxCharsPerFilepath, "%s-humidity-adc-trace.csv", arguments->measurementsPathPrefix);

		if ((ret < 1) || (ret >= kCommonConstantMaxCharsPerFilepath))
		{
			fprintf(stderr, "Failed to create filename for loading from %s-humidity-adc-trace.csv", arguments->measurementsPathPrefix);

			return kCommonConstantReturnTypeError;
		}

		if (readInputFloatDistributionsFromCSV(filename, inputHumidityADCTraceCSVHeader, humidityRawADCValue, 1) != kCommonConstantReturnTypeSuccess)
		{
			return kCommonConstantReturnTypeError;
		}
	}
	else
	{
		*temperatureRawADCValue = arguments->temperatureRawADCValue;
		*pressureRawADCValue = arguments->pressureRawADCValue;
		*humidityRawADCValue = arguments->humidityRawADCValue;
	}

	return kCommonConstantReturnTypeSuccess;
}

CommonConstantReturnType
loadNthFloatFromPath(const char *  filename, uint64_t chosenRowNumber, float *  returnValuePtr)
{
	FILE *		fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error opening input file");

		return kCommonConstantReturnTypeError;
	}

	for (int i = 0; i <= chosenRowNumber; i++)
	{
		fscanf(fp, "%f\n", returnValuePtr);
	}

	return kCommonConstantReturnTypeSuccess;
}

/**
 *	@brief	Populates a `JSONVariable` struct
 *
 *	@param	jsonVariable			: Pointer to the `JSONVariable` struct to populate.
 *	@param	outputVariableValues		: The array of values for the output variable from which the `JSONVariable` struct will take values.
 *	@param	outputVariableDescription	: Description of the output variable from which the `JSONVariable` struct will take its description.
 *	@param	outputSelect			: Index of the selected output.
 *	@param	numberOfOutputVariableValues	: The number of values in `outputVariableValues`.
 */
void
populateJSONVariableStruct(
	JSONVariable *		jsonVariable,
	float *			outputVariableValues,
	const char *		outputVariableDescription,
	OutputDistributionIndex	outputSelect,
	size_t			numberOfOutputVariableValues)
{
	snprintf(jsonVariable->variableSymbol, kCommonConstantMaxCharsPerJSONVariableSymbol, "outputVariables[%u]", outputSelect);
	snprintf(jsonVariable->variableDescription, kCommonConstantMaxCharsPerJSONVariableDescription, "%s", outputVariableDescription);
	jsonVariable->values = (JSONVariablePointer){ .asFloat = outputVariableValues };
	jsonVariable->type = kJSONVariableTypeFloat;
	jsonVariable->size = numberOfOutputVariableValues;

	return;
}

/**
 *	@brief	Determine the index range of selected outputs.
 *
 *	@param	arguments			: Pointer to command line arguments struct.
 *	@param	pointerToOutputSelectLowerBound	: Pointer to lower bound index.
 *	@param	pointerToOutputSelectUpperBound	: Pointer to upper bound index.
 */
void
determineIndexRangeOfSelectedOutputs(
	CommandLineArguments *		arguments,
	OutputDistributionIndex *	pointerToOutputSelectLowerBound,
	OutputDistributionIndex *	pointerToOutputSelectUpperBound)
{
	/*
	 *	If outputSelect is equal to `kOutputDistributionIndexMax`, index range is the full range.
	 */
	if (arguments->common.outputSelect == kOutputDistributionIndexMax)
	{
		*pointerToOutputSelectLowerBound = (OutputDistributionIndex)0;
		*pointerToOutputSelectUpperBound = kOutputDistributionIndexMax;
	}
	/*
	 *	Else, index range lower bound is the index of the selected output and the length of range is 1 (single output selected).
	 */
	else
	{
		*pointerToOutputSelectLowerBound = arguments->common.outputSelect;
		*pointerToOutputSelectUpperBound = *pointerToOutputSelectLowerBound + 1;
	}

	return;
}

/**
 *	@brief	Populate and print JSON variables.
 *
 *	@param	jsonVariables			: Array of `JSONVariable` structs to populate and print.
 *	@param	arguments			: Pointer to command line arguments struct.
 *	@param	outputVariables			: The output variables.
 *	@param	outputVariableDescriptions	: Descriptions of output variables from which the array of `JSONVariable` structs will take their descriptions.
 *	@param	monteCarloOutputSamples		: Monte Carlo output samples that will populate `JSONVariable` struct values if in Monte Carlo mode.
 */
void
populateAndPrintJSONVariables(
	JSONVariable *		jsonVariables,
	CommandLineArguments *	arguments,
	float *			outputVariables,
	const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
	float			monteCarloOutputSamples[])
{
	OutputDistributionIndex	outputSelectLowerBound;
	OutputDistributionIndex	outputSelectUpperBound;

	determineIndexRangeOfSelectedOutputs(
		arguments,
		&outputSelectLowerBound,
		&outputSelectUpperBound);

	for (OutputDistributionIndex outputSelect = outputSelectLowerBound; outputSelect < outputSelectUpperBound; outputSelect++)
	{
		/*
		 *	If in Monte Carlo mode, `pointerToOutputVariable` points to the beginning of the `monteCarloOutputSamples` array.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` is the length of the `monteCarloOutputSamples` array.
		 *	Else, it points to the entry of the `outputVariables` to be used.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` equals 1.
		 */
		float *  pointerToOutputVariable = arguments->common.isMonteCarloMode ? monteCarloOutputSamples : &outputVariables[outputSelect];

		populateJSONVariableStruct(
			&jsonVariables[outputSelect],
			pointerToOutputVariable,
			outputVariableDescriptions[outputSelect],
			outputSelect,
			arguments->common.numberOfMonteCarloIterations);
	}

	printJSONVariables(
		&jsonVariables[outputSelectLowerBound],
		outputSelectUpperBound - outputSelectLowerBound,
		"Output variables");

	return;
}

/**
 *	@brief	Print human-consumable output.
 *
 *	@param	arguments			: Pointer to command line arguments struct.
 *	@param	outputVariables			: The output variables.
 *	@param	outputNames			: Names of the output variables to print.
 *	@param	outputVariableDescriptions	: Descriptions of output variables to print.
 *	@param	monteCarloOutputSamples		: Monte Carlo output samples that will populate JSON struct values if in Monte Carlo mode.
 */
void
printHumanConsumableOutput(
	CommandLineArguments *	arguments,
	float *			outputVariables,
	const char *		outputVariableNames[kOutputDistributionIndexMax],
	const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
	float			monteCarloOutputSamples[])
{
	OutputDistributionIndex	outputSelectLowerBound;
	OutputDistributionIndex	outputSelectUpperBound;

	determineIndexRangeOfSelectedOutputs(
		arguments,
		&outputSelectLowerBound,
		&outputSelectUpperBound);

	for (OutputDistributionIndex outputSelect = outputSelectLowerBound; outputSelect < outputSelectUpperBound; outputSelect++)
	{
		/*
		 *	If in Monte Carlo mode, `pointerToOutputVariable` points to the beginning of the `monteCarloOutputSamples` array.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` is the length of the `monteCarloOutputSamples` array.
		 *	Else, it points to the entry of the `outputVariables` to be used.
		 *	In this case, `arguments.common.numberOfMonteCarloIterations` equals 1.
		 */
		float *  pointerToValueToPrint = arguments->common.isMonteCarloMode ? monteCarloOutputSamples : &outputVariables[outputSelect];

		for (size_t i = 0; i < arguments->common.numberOfMonteCarloIterations; ++i)
		{
			printf("%s is %f.\n", outputVariableDescriptions[outputSelect], *pointerToValueToPrint);
			pointerToValueToPrint++;
		}
	}

	return;
}
