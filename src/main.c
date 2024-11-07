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

#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uxhw.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "bme680.h"
#include "utilities.h"
#include "common.h"


/**
 *	@brief	Calculate the output of the BME680 conversion routines.
 *
 *	@param	arguments	: Pointer to command-line arguments struct.
 *	@param	inputVariables	: The input variables.
 *	@param	outputVariables	: The output variables.
 */
static void
calculateBME680ConversionRoutines(
	CommandLineArguments *	arguments,
	float *			inputVariables,
	float *			outputVariables,
	float *			temperatureParameters,
	float *			pressureParameters,
	float *			humidityParameters)
{
	bool	calculateAllOutputs = (arguments->common.outputSelect == kOutputDistributionIndexMax);

	/*
	 *	Not guarded because we need temperature calculation in all cases.
	 */
	outputVariables[kOutputDistributionIndexForTemperature] = calc_temperature(
									inputVariables[kInputDistributionIndexForTemperatureRawADCValue],
									temperatureParameters[0],
									temperatureParameters[1],
									temperatureParameters[2]);

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputDistributionIndexForPressure))
	{
		outputVariables[kOutputDistributionIndexForPressure] = calc_pressure(
										inputVariables[kInputDistributionIndexForPressureRawADCValue],
										outputVariables[kOutputDistributionIndexForTemperature],
										pressureParameters[0],
										pressureParameters[1],
										pressureParameters[2],
										pressureParameters[3],
										pressureParameters[4],
										pressureParameters[5],
										pressureParameters[6],
										pressureParameters[7],
										pressureParameters[8],
										pressureParameters[9]) / 1000;
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputDistributionIndexForHumidity))
	{
		outputVariables[kOutputDistributionIndexForHumidity] = calc_humidity(
										inputVariables[kInputDistributionIndexForHumidityRawADCValue],
										outputVariables[kOutputDistributionIndexForTemperature],
										humidityParameters[0],
										humidityParameters[1],
										humidityParameters[2],
										humidityParameters[3],
										humidityParameters[4],
										humidityParameters[5],
										humidityParameters[6]);
	}

	return;
}

/**
 *	@brief	Set distributions for input variables via UxHw calls.
 *
 *	@param	inputVariables	: The input variables.
 */
static void
setInputVariablesViaUxHwCalls(float *  inputVariables)
{
	inputVariables[kInputDistributionIndexForTemperatureRawADCValue] = UxHwFloatUniformDist(
								kBME680ConstantsTemperatureRawADCValueLowerBound,
								kBME680ConstantsTemperatureRawADCValueUpperBound);
	inputVariables[kInputDistributionIndexForPressureRawADCValue] = UxHwFloatUniformDist(
								kBME680ConstantsPressureRawADCValueLowerBound,
								kBME680ConstantsPressureRawADCValueUpperBound);
	inputVariables[kInputDistributionIndexForHumidityRawADCValue] = UxHwFloatUniformDist(
								kBME680ConstantsHumidityRawADCValueLowerBound,
								kBME680ConstantshumidityRawADCValueUpperBound);

	return;
}

int
main(int argc, char *  argv[])
{
	CommandLineArguments	arguments = (CommandLineArguments) {0};
	float			temperatureParameters[kBME680ConstantsNumberOfTemperatureParameters];
	float			pressureParameters[kBME680ConstantsNumberOfPressureParameters];
	float			humidityParameters[kBME680ConstantsNumberOfHumidityParameters];
	/*
	 *	Variable `inputVariables[0]` corresponds to the raw ADC value for the temperature reading.
	 *	Variable `inputVariables[1]` corresponds to the raw ADC value for the pressure reading.
	 *	Variable `inputVariables[2]` corresponds to the raw ADC value for the humidity reading.
	 */
	float			inputVariables[kInputDistributionIndexMax];
	/*
	 *	Variable `outputVariables[0]` corresponds to the converted temperature reading.
	 *	Variable `outputVariables[1]` corresponds to the converted pressure reading.
	 *	Variable `outputVariables[2]` corresponds to the converted humidity reading.
	 */
	float			outputVariables[kOutputDistributionIndexMax];
	const char *		outputVariableNames[kOutputDistributionIndexMax] =
				{
					"temperature",
					"pressure",
					"humidity"
				};
	const char *		outputVariableDescriptions[kOutputDistributionIndexMax] =
				{
					"Temperature reading with uncertainty of BME680 sensor ",
					"Pressure reading with uncertainty of BME680 sensor",
					"Humidity reading with uncertainty of BME680 sensor"
				};
	float			benchmarkOutput;
	float *			monteCarloOutputSamples = NULL;
	MeanAndVariance		monteCarloOutputMeanAndVariance = {0};
	clock_t			start = 0;
	clock_t			end = 0;
	float			cpuTimeUsedInSeconds;

	/*
	 *	Get command line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	/*
	 *	Load inputs.
	 */
	if (loadInputs(
			&arguments,
			temperatureParameters,
			pressureParameters,
			humidityParameters,
			&inputVariables[kInputDistributionIndexForTemperatureRawADCValue],
			&inputVariables[kInputDistributionIndexForPressureRawADCValue],
			&inputVariables[kInputDistributionIndexForHumidityRawADCValue]) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	/*
	 *	Allocate for `monteCarloOutputSamples` if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples = (float *) checkedMalloc(
								arguments.common.numberOfMonteCarloIterations * sizeof(float),
								__FILE__,
								__LINE__);
	}

	/*
	 *	Start timing if timing is enabled or in benchmarking mode.
	 */
	if ((arguments.common.isTimingEnabled) || (arguments.common.isBenchmarkingMode))
	{
		start = clock();
	}

	/*
	 *	Execute process kernel in a loop. The size of loop is 1 unless in Monte Carlo mode.
	 */
	for (size_t i = 0; i < arguments.common.numberOfMonteCarloIterations; ++i)
	{
		/*
		 *	Set inputs via UxHw calls if input from file is not enabled.
		 */
		if (!arguments.useInputADCFiles)
		{
			setInputVariablesViaUxHwCalls(inputVariables);
		}

		/*
		 *	Execute conversion routine.
		 */
		calculateBME680ConversionRoutines(&arguments,
				inputVariables,
				outputVariables,
				temperatureParameters,
				pressureParameters,
				humidityParameters);

		/*
		 *	If in Monte Carlo mode, populate `monteCarloOutputSamples`.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[i] = outputVariables[arguments.common.outputSelect];
		}
		/*
		 *	Else, if in benchmarking mode, populate `benchmarkOutput`.
		 */
		else if (arguments.common.isBenchmarkingMode)
		{
			benchmarkOutput = outputVariables[arguments.common.outputSelect];
		}
	}

	/*
	 *	If not doing Laplace version, then approximate the cost of the third phase of
	 *	Monte Carlo (post-processing), by calculating the mean and variance.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputMeanAndVariance = calculateMeanAndVarianceOfFloatSamples(
								monteCarloOutputSamples,
								arguments.common.numberOfMonteCarloIterations);
		benchmarkOutput = monteCarloOutputMeanAndVariance.mean;
	}

	/*
	 *	Stop timing if timing is enabled or in benchmarking mode.
	 */
	if ((arguments.common.isTimingEnabled) || (arguments.common.isBenchmarkingMode))
	{
		end = clock();
		cpuTimeUsedInSeconds = ((double)(end - start)) / CLOCKS_PER_SEC;
	}

	/*
	 *	If in benchmarking mode, print timing result in a special format:
	 *		(1) Benchmark output (for calculating Wasserstein distance to reference)
	 *		(2) Time in microseconds
	 */
	if (arguments.common.isBenchmarkingMode)
	{
		printf("%lf %" PRIu64 "\n", benchmarkOutput, (uint64_t)(cpuTimeUsedInSeconds * 1000000));
	}
	/*
	 *	If not in benchmarking mode...
	 */
	else
	{
		/*
		 *	Print json outputs if in JSON output mode.
		 */
		if (arguments.common.isOutputJSONMode)
		{
			JSONVariable	jsonVariables[kOutputDistributionIndexMax];

			populateAndPrintJSONVariables(
				jsonVariables,
				&arguments,
				outputVariables,
				outputVariableDescriptions,
				monteCarloOutputSamples);
		}
		/*
		 *	Print human-consumable output if not in JSON output mode.
		 */
		else
		{
			printHumanConsumableOutput(
				&arguments,
				outputVariables,
				outputVariableNames,
				outputVariableDescriptions,
				monteCarloOutputSamples);
		}

		/*
		 *	Print timing if timing is enabled.
		 */
		if (arguments.common.isTimingEnabled)
		{
			printf("\nCPU time used: %lf seconds\n", cpuTimeUsedInSeconds);
		}
	}

	/*
	 *	Save Monte Carlo data to "data.out" if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloFloatDataToDataDotOutFile(
			monteCarloOutputSamples,
			(uint64_t)(cpuTimeUsedInSeconds * 1000000),
			arguments.common.numberOfMonteCarloIterations);
	}
	/*
	 *	Save outputs to file if not in Monte Carlo mode and write to file is enabled.
	 */
	else
	{
		if (arguments.common.isWriteToFileEnabled)
		{
			if(writeOutputFloatDistributionsToCSV(
				arguments.common.outputFilePath,
				outputVariables,
				outputVariableNames,
				kOutputDistributionIndexMax))
			{
				fprintf(stderr, "Error: Could not write to output CSV file \"%s\".\n", arguments.common.outputFilePath);
				return EXIT_FAILURE;
			}
		}
	}

	/*
	 *	Free allocations
	 */
	if (arguments.common.isMonteCarloMode)
	{
		free(monteCarloOutputSamples);
	}

	return EXIT_SUCCESS;
}
